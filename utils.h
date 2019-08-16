#ifndef UTILS_H
#define UTILS_H
#include <tuple>


/////////////TUPLE ITERATION/////////////

template<class F, class...Ts, std::size_t...Is>
void iterateTuple(std::tuple<Ts...> & tuple, F && func, std::index_sequence<Is...>){
    using expander = int[];
    (void)expander { 0, ((void)func(std::get<Is>(tuple)), 0)... };
}

template<class F, class...Ts>
void iterateTuple(std::tuple<Ts...> & tuple, F && func){
    iterateTuple(tuple, std::move(func), std::make_index_sequence<sizeof...(Ts)>());
}


/////////////FIND TYPE INDEX IN TUPLE//////

template<std::size_t CurrIndex, class T, class TupleT, typename TTT=void >
struct tuple_first_type_index;

template<std::size_t CurrIndex, class T,  class Head, class... Tail>
struct tuple_first_type_index<CurrIndex, T, std::tuple<Head, Tail...>, std::enable_if_t<!std::is_same_v<Head, T>>>
    : tuple_first_type_index<CurrIndex+1, T, std::tuple<Tail...>> {};

template<std::size_t CurrIndex, class T,  class Head, class... Tail >
struct tuple_first_type_index<CurrIndex, T, std::tuple<Head, Tail...>, std::enable_if_t<std::is_same_v<Head, T>>> {
    static constexpr std::size_t value = CurrIndex;
};

template <class T, class TupleT>
constexpr std::size_t tuple_first_type_index_v = tuple_first_type_index<0, T, TupleT>::value;


/////////////EXTRACT TYPES SUBTUPLE FROM TUPLE////////////

template<std::size_t FromI, std::size_t ToI, class TupleT, class Sequence, typename TTT=void >
struct tuple_types_interval {};

template<std::size_t FromI, std::size_t ToI, class TupleT, std::size_t ...Is>
struct tuple_types_interval<FromI, ToI, TupleT, std::index_sequence<Is...>, std::enable_if_t<sizeof... (Is) < ToI-FromI>>
    : tuple_types_interval<FromI, ToI, TupleT, std::index_sequence<Is..., FromI+sizeof... (Is)>> {};

template<std::size_t FromI, std::size_t ToI, class TupleT, std::size_t ...Is>
struct tuple_types_interval<FromI, ToI, TupleT, std::index_sequence<Is...>, std::enable_if_t<sizeof... (Is) == ToI-FromI>> {
    using type = std::tuple<std::tuple_element_t<Is,TupleT >...>;
};

template<std::size_t FromI, std::size_t ToI, class TupleT>
using tuple_types_interval_t= typename tuple_types_interval<FromI, ToI, TupleT, std::index_sequence<>>::type;


/////////////CONCAT TUPLES/////////////

template<typename ... input_t>
using tuple_cat_t=
decltype(std::tuple_cat(
    std::declval<input_t>()...
));


/////////////TAKE TYPE FROM TUPLE BY INDEX

template<std::size_t Index, class TupleT>
struct tuple_type_extracter {};

template<std::size_t Index, class ...TupleTypes>
struct tuple_type_extracter<Index, std::tuple<TupleTypes...>> {
    using Tpl = std::tuple<TupleTypes...>;
    using ExtractedT = std::tuple_element_t<Index, Tpl>;
    using RestT = tuple_cat_t<tuple_types_interval_t<0, Index, Tpl>, tuple_types_interval_t<Index+1, sizeof... (TupleTypes), Tpl>>;
};


/////////////FIND MINUMUM TYPE by ::value MEMBER IN TUPLE

template<class ElemsTuple, typename TTT = void>
struct min_finder {};

template<class Elem>
struct min_finder<std::tuple<Elem>> {
    using type = Elem;
    static constexpr auto value = Elem::value;
};

template<class ElemFirst, class ElemSecond, class ... ElemsRest>
struct min_finder<std::tuple<ElemFirst, ElemSecond, ElemsRest...>, std::enable_if_t<ElemFirst::value < ElemSecond::value>>:
        min_finder<std::tuple<ElemFirst, ElemsRest...>> {};

template<class ElemFirst, class ElemSecond, class ... ElemsRest>
struct min_finder<std::tuple<ElemFirst, ElemSecond, ElemsRest...>, std::enable_if_t<ElemFirst::value >= ElemSecond::value>>:
        min_finder<std::tuple<ElemSecond, ElemsRest...>> {};


template<class Tuple>
using min_finder_t =  typename min_finder<Tuple>::type;


/////////////SORT TYPES TUPLE ASCENDING BY ::value MEMBER

template<class TupleT, class TupleSorted,  typename TTT=void >
struct sorter {};

template <class TupleT>
struct MinRestExtractor{
    using Min = min_finder_t<TupleT>;
    using Rest = typename tuple_type_extracter<tuple_first_type_index_v<Min, TupleT>, TupleT>::RestT;
};

template<class TupleT, class ...Sorted>
struct sorter<TupleT,  std::tuple<Sorted...>, std::enable_if_t<0<std::tuple_size_v<TupleT>>>
    : sorter<typename MinRestExtractor<TupleT>::Rest, std::tuple<Sorted..., typename MinRestExtractor<TupleT>::Min>> {};

template<class TupleT, class ...Sorted>
struct sorter<TupleT, std::tuple<Sorted...>, std::enable_if_t<0 == std::tuple_size_v<TupleT>>> {
    using type = std::tuple<Sorted...>;
};

template<class TupleT>
using sorted_tuple_t = typename sorter<TupleT, std::tuple<>>::type;


/////////////REMOVE DUPLICATES IN SORTED TUPLE


template<class SortedTupleT, class DupsFreeTupleT,  typename TTT=void >
struct unique_only_getter {};

template<class SortedTupleHeadT, class ...SortedTupleOtherTs>
struct unique_only_getter<std::tuple<SortedTupleHeadT, SortedTupleOtherTs...>,  std::tuple<>,
        std::enable_if_t < 0 < sizeof... (SortedTupleOtherTs)  >>
    : unique_only_getter<std::tuple<SortedTupleOtherTs...>, std::tuple<SortedTupleHeadT>> {};


template<class SortedTupleHeadT, class DupsFreeHeadT, class ...DupsFreeTs>
struct unique_only_getter<std::tuple<SortedTupleHeadT>,  std::tuple<DupsFreeHeadT, DupsFreeTs...>,
        std::enable_if_t < !std::is_same_v<SortedTupleHeadT, DupsFreeHeadT > > > {
    using type = std::tuple<SortedTupleHeadT, DupsFreeHeadT, DupsFreeTs...>;
};

template<class SortedTupleHeadT, class DupsFreeHeadT, class ...DupsFreeTs>
struct unique_only_getter<std::tuple<SortedTupleHeadT>,  std::tuple<DupsFreeHeadT, DupsFreeTs...>,
        std::enable_if_t < std::is_same_v<SortedTupleHeadT, DupsFreeHeadT > > > {
    using type = std::tuple<DupsFreeHeadT, DupsFreeTs...>;
};

template<class SortedTupleHeadT, class ...SortedTupleOtherTs, class DupsFreeHeadT, class ...DupsFreeTs>
struct unique_only_getter<std::tuple<SortedTupleHeadT, SortedTupleOtherTs...>,  std::tuple<DupsFreeHeadT, DupsFreeTs...>,
        std::enable_if_t < !std::is_same_v<SortedTupleHeadT, DupsFreeHeadT > >>
    : unique_only_getter<std::tuple<SortedTupleOtherTs...>, std::tuple<SortedTupleHeadT, DupsFreeHeadT, DupsFreeTs...>> {};


template<class SortedTupleHeadT, class ...SortedTupleOtherTs, class DupsFreeHeadT, class ...DupsFreeTs>
struct unique_only_getter<std::tuple<SortedTupleHeadT, SortedTupleOtherTs...>,  std::tuple<DupsFreeHeadT, DupsFreeTs...>,
        std::enable_if_t < std::is_same_v<SortedTupleHeadT,  DupsFreeHeadT> >>
    : unique_only_getter<std::tuple<SortedTupleOtherTs...>, std::tuple<DupsFreeHeadT, DupsFreeTs...> > {};



template<class TupleT>
using unique_only_getter_t = typename unique_only_getter<sorted_tuple_t<TupleT>, std::tuple<>>::type;
#endif // UTILS_H
