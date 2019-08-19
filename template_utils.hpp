#ifndef TEMPLATE_UTILS_H
#define TEMPLATE_UTILS_H
#include <tuple>
#include <array>

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

template<class F, class...Ts, std::size_t...Is, class ...Args>
void iterateTuple(const std::tuple<Ts...> & tuple, F && func, std::index_sequence<Is...>, Args...args){
    using expander = int[];
    (void)expander { 0, ((void)func(std::get<Is>(tuple), args...), 0)... };
}

template<class F, class...Ts, class ...Args>
void iterateTuple(const std::tuple<Ts...> & tuple, F && func, Args...args){
    iterateTuple(tuple, std::move(func), std::make_index_sequence<sizeof...(Ts)>(), args...);
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

static_assert (tuple_first_type_index_v<int, std::tuple<float, bool, int, int>> == 2, "works");


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

static_assert(std::is_same_v<std::tuple<float>, tuple_types_interval_t<1,2,  std::tuple<int, float, char, bool>>>, "dddd");
static_assert(std::is_same_v<std::tuple<float, char>, tuple_types_interval_t<1,3,  std::tuple<int, float, char, bool>>>, "dddd");
static_assert(std::is_same_v<std::tuple<float, char, bool>, tuple_types_interval_t<1,4,  std::tuple<int, float, char, bool>>>, "dddd");
static_assert(std::is_same_v<std::tuple<int, float>, tuple_types_interval_t<0,2,  std::tuple<int, float, char, bool>>>, "dddd");


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

using extr_test = tuple_type_extracter<2, std::tuple<int, float, char>>;
static_assert(std::is_same_v<extr_test::ExtractedT, char>, "dddd");
static_assert(std::is_same_v<extr_test::RestT, std::tuple<int, float>>, "dddd");

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
template<class Tuple>
static constexpr auto min_finder_v =  min_finder<Tuple>::value;


static_assert(min_finder<std::tuple<std::integral_constant<int, 5>, std::integral_constant<int, 4>,  std::integral_constant<int, 8>>>::value == 4, "dddd");

/////////////FIND MAXIMUM TYPE by ::value MEMBER IN TUPLE

template<class ElemsTuple, typename TTT = void>
struct max_finder {};

template<class Elem>
struct max_finder<std::tuple<Elem>> {
    using type = Elem;
    static constexpr auto value = Elem::value;
};

template<class ElemFirst, class ElemSecond, class ... ElemsRest>
struct max_finder<std::tuple<ElemFirst, ElemSecond, ElemsRest...>, std::enable_if_t<(ElemFirst::value > ElemSecond::value)>>:
        max_finder<std::tuple<ElemFirst, ElemsRest...>> {};

template<class ElemFirst, class ElemSecond, class ... ElemsRest>
struct max_finder<std::tuple<ElemFirst, ElemSecond, ElemsRest...>, std::enable_if_t<ElemFirst::value <= ElemSecond::value>>:
        max_finder<std::tuple<ElemSecond, ElemsRest...>> {};

template<class Tuple>
using max_finder_t =  typename max_finder<Tuple>::type;
template<class Tuple>
static constexpr auto max_finder_v =  max_finder<Tuple>::value;

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


template<int V>
using C = std::integral_constant<int, V>;
using to_sort = std::tuple<
                C<5>,
                C<4>,
                C<8>,
                C<4>,
                C<5>,
                C<2>,
                C<8>
>;


using sorted_t = sorted_tuple_t<to_sort>;
static_assert(std::is_same_v<sorted_t,  std::tuple<
              C<2>,
              C<4>,
              C<4>,
              C<5>,
              C<5>,
              C<8>,
              C<8>
              >>, "Sorted not works");


/////////////REMOVE DUPLICATES IN SORTED TUPLE


template<class SortedTupleT, class DupsFreeTupleT,  typename TTT=void >
struct unique_only_getter {};

template<>
struct unique_only_getter<std::tuple<>,  std::tuple<>>{
    using type = std::tuple<>;
};

template<class SortedTupleHeadT>
struct unique_only_getter<std::tuple<SortedTupleHeadT>,  std::tuple<>>{
    using type = std::tuple<SortedTupleHeadT>;
};

template<class SortedTupleHeadT, class ...SortedTupleOtherTs>
struct unique_only_getter<std::tuple<SortedTupleHeadT, SortedTupleOtherTs...>,  std::tuple<>,
        std::enable_if_t < 0 < sizeof... (SortedTupleOtherTs)  >>
    : unique_only_getter<std::tuple<SortedTupleOtherTs...>, std::tuple<SortedTupleHeadT>> {};


template<class SortedTupleHeadT, class DupsFreeHeadT, class ...DupsFreeTs>
struct unique_only_getter<std::tuple<SortedTupleHeadT>,  std::tuple<DupsFreeHeadT, DupsFreeTs...>,
        std::enable_if_t < SortedTupleHeadT::value != DupsFreeHeadT::value > > {
    using type = std::tuple<SortedTupleHeadT, DupsFreeHeadT, DupsFreeTs...>;
};

template<class SortedTupleHeadT, class DupsFreeHeadT, class ...DupsFreeTs>
struct unique_only_getter<std::tuple<SortedTupleHeadT>,  std::tuple<DupsFreeHeadT, DupsFreeTs...>,
        std::enable_if_t < SortedTupleHeadT::value == DupsFreeHeadT::value > > {
    using type = std::tuple<DupsFreeHeadT, DupsFreeTs...>;
};

template<class SortedTupleHeadT, class ...SortedTupleOtherTs, class DupsFreeHeadT, class ...DupsFreeTs>
struct unique_only_getter<std::tuple<SortedTupleHeadT, SortedTupleOtherTs...>,  std::tuple<DupsFreeHeadT, DupsFreeTs...>,
        std::enable_if_t < SortedTupleHeadT::value != DupsFreeHeadT::value >>
    : unique_only_getter<std::tuple<SortedTupleOtherTs...>, std::tuple<SortedTupleHeadT, DupsFreeHeadT, DupsFreeTs...>> {};


template<class SortedTupleHeadT, class ...SortedTupleOtherTs, class DupsFreeHeadT, class ...DupsFreeTs>
struct unique_only_getter<std::tuple<SortedTupleHeadT, SortedTupleOtherTs...>,  std::tuple<DupsFreeHeadT, DupsFreeTs...>,
        std::enable_if_t < SortedTupleHeadT::value ==   DupsFreeHeadT::value>>
    : unique_only_getter<std::tuple<SortedTupleOtherTs...>, std::tuple<DupsFreeHeadT, DupsFreeTs...> > {};



template<class TupleT>
using unique_only_getter_t = typename unique_only_getter<sorted_tuple_t<TupleT>, std::tuple<>>::type;

using uniques = unique_only_getter_t<to_sort>;

static_assert(std::tuple_size_v<uniques> == 4);
static_assert(std::is_same_v<uniques,  std::tuple<
              C<8>,
              C<5>,
              C<4>,
              C<2>
              >>, "Distinct fail");

/////////////FILTER TYPES WITH equal ::value to given

//add condition function option
template<class ValT, ValT val, class InputTuple, class ResultT, typename TTT=void >
struct by_value_filterer {};

template<class ValT, ValT val >
struct by_value_filterer<ValT, val, std::tuple<>, std::tuple<> > {
    using type = std::tuple<>;
};


template<class ValT, ValT val, class InputT, class ...ResultTypes >
struct by_value_filterer<ValT, val, std::tuple<InputT>, std::tuple<ResultTypes...>,
        std::enable_if_t<InputT::value == val> > {
    using type = std::tuple<ResultTypes..., InputT>;
};

template<class ValT, ValT val, class InputT, class ...ResultTypes >
struct by_value_filterer<ValT, val, std::tuple<InputT>, std::tuple<ResultTypes...>,
        std::enable_if_t<InputT::value != val> > {
    using type = std::tuple<ResultTypes...>;
};


template<class ValT, ValT val, class InputT, class ...OtherInputTs, class ...ResultTypes >
struct by_value_filterer<ValT, val, std::tuple<InputT, OtherInputTs...>, std::tuple<ResultTypes...>,
        std::enable_if_t<InputT::value == val> > :
        by_value_filterer<ValT, val, std::tuple<OtherInputTs...>, std::tuple<ResultTypes..., InputT> >
{

};

template<class ValT, ValT val, class InputT, class ...OtherInputTs, class ...ResultTypes >
struct by_value_filterer<ValT, val, std::tuple<InputT, OtherInputTs...>, std::tuple<ResultTypes...>,
        std::enable_if_t<InputT::value != val> > :
        by_value_filterer<ValT, val, std::tuple<OtherInputTs...>, std::tuple<ResultTypes...> >
{

};

template <class ValT, ValT val, class InputT>
using by_value_filterer_t = typename by_value_filterer<ValT, val, InputT, std::tuple<>>::type;

using filtered = by_value_filterer_t<int, 4, to_sort>;
static_assert(std::tuple_size_v<filtered> == 2);
static_assert(std::is_same_v<filtered,  std::tuple<
              C<4>,
              C<4>
              >>, "Filter fail");

/////////////MINMAX
template <class T>
static constexpr auto StaticMin(const T & v) {
    return v;
}

template <class FirstT, class SecondT, class ...OtherT>
static constexpr auto StaticMin(const FirstT & f, const SecondT & s, const OtherT & ... others) {
    if(f < s) return StaticMin(f, others...);
    return StaticMin(s, others...);
}


template <class T>
static constexpr auto StaticMax(const T & v) {
    return v;
}

template <class FirstT, class SecondT, class ...OtherT>
static constexpr auto StaticMax(const FirstT & f, const SecondT & s, const OtherT & ... others) {
    if(f > s) return StaticMax(f, others...);
    return StaticMax(s, others...);
}


template <class ...OtherT>
static constexpr auto StaticMax(const std::tuple<OtherT...> /*args*/) {
  return StaticMax(OtherT()...);
}

/////////////REPACK WITH INDEXES//////////////
template< typename ItemType, std::size_t Index>
struct IndexedType {
    using ItemT = ItemType;
    static constexpr std::size_t I = Index;
};
template <std::size_t I, typename Out, typename ... In>
struct indexer{

};

template <std::size_t I, typename ... Items>
struct indexer<I, std::tuple<Items...>>{
    using type = std::tuple<Items...>;
};

template <std::size_t I, typename ... Items, typename ...SrcItems>
struct indexer<I, std::tuple<Items...>, std::tuple<SrcItems...>>:
        indexer<I, std::tuple<Items...>, SrcItems...>
{
};


template <std::size_t I, typename ...IndexedTypes, typename FirstItem,  typename ... OtherItems>
struct indexer<I, std::tuple<IndexedTypes...>, FirstItem,  OtherItems...>:
        indexer<I + 1, std::tuple<IndexedTypes..., IndexedType<FirstItem, I>>, OtherItems...>{
};


template <typename ... SrcT>
using indexed_types_t = typename indexer<0, std::tuple<>, SrcT...>::type;

using indexed_types_test = indexed_types_t<std::tuple<int, float, double>>;

static_assert (std::tuple_element_t<0, indexed_types_test>::I == 0);
static_assert (std::tuple_element_t<1, indexed_types_test>::I == 1);
static_assert (std::tuple_element_t<2, indexed_types_test>::I == 2);

static_assert (std::is_same_v<int, std::tuple_element_t<0, indexed_types_test>::ItemT>);
static_assert (std::is_same_v<float, std::tuple_element_t<1, indexed_types_test>::ItemT>);
static_assert (std::is_same_v<double, std::tuple_element_t<2, indexed_types_test>::ItemT>);

/////////////VALUE extractor

//template <template <typename> typename ValueExtractorT>
//struct
#endif // TEMPLATE_UTILS_H
