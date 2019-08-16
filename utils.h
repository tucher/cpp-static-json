#ifndef UTILS_H
#define UTILS_H
#include <tuple>

template<class F, class...Ts, std::size_t...Is>
void iterateTuple(std::tuple<Ts...> & tuple, F && func, std::index_sequence<Is...>){
    using expander = int[];
    (void)expander { 0, ((void)func(std::get<Is>(tuple)), 0)... };
}

template<class F, class...Ts>
void iterateTuple(std::tuple<Ts...> & tuple, F && func){
    iterateTuple(tuple, std::move(func), std::make_index_sequence<sizeof...(Ts)>());
}


template <class T, class ...OtherT>
static constexpr bool IsInList(const T & f, const OtherT & ... others) {
    return (false || ... || (f==others));
}

template <std::size_t ... Is, class ...T>
static constexpr auto SplitTupleIn2_h1(std::tuple<T...> tuple, std::index_sequence<Is...>) {
    constexpr bool odd = (sizeof... (T)) % 2 != 0;
    constexpr std::size_t N_2 = (sizeof... (T)) / 2;
    if constexpr(!odd)
        return std::make_pair(std::make_tuple(std::get<Is>(tuple)...), std::make_tuple(std::get<Is+N_2>(tuple)...));
    else
        return std::make_pair(std::make_tuple(std::get<0>(tuple), std::get<Is+1>(tuple)...), std::make_tuple(std::get<Is+N_2+1>(tuple)...));
}


template <class ...T>
static constexpr auto SplitTupleIn2(std::tuple<T...> tuple) {
    return SplitTupleIn2_h1(tuple, std::make_index_sequence<sizeof... (T)/2>());
}

template <typename ...TupleLeftTypes, typename ...TupleRightTypes>
static constexpr auto MergeSortedLists(std::tuple<TupleLeftTypes...> tupleLeft, std::tuple<TupleRightTypes...> tupleRight) {
    if constexpr (sizeof... (TupleLeftTypes) == 0) {
        return tupleRight;
    } else if constexpr(sizeof... (TupleRightTypes) == 0) {
        return tupleLeft;
    }
    return std::make_tuple(true);
}

//template <typename ...TupleRightTypes>
//static constexpr auto MergeSortedLists(std::tuple<> tupleLeft, std::tuple<TupleRightTypes...> tupleRight) {
//    return tupleRight;
//}

//template <typename ...TupleLeftTypes>
//static constexpr auto MergeSortedLists(std::tuple<TupleLeftTypes...> tupleLeft, std::tuple<> tupleRight) {
//    return tupleLeft;
//}


template<std::size_t CurrIndex, class T, class TupleT, typename TTT=void >
struct tuple_first_type_index;

// recursive case
template<std::size_t CurrIndex, class T,  class Head, class... Tail>
struct tuple_first_type_index<CurrIndex, T, std::tuple<Head, Tail...>, std::enable_if_t<!std::is_same_v<Head, T>>>
    : tuple_first_type_index<CurrIndex+1, T, std::tuple<Tail...>> {

};

// base case
template<std::size_t CurrIndex, class T,  class Head, class... Tail >
struct tuple_first_type_index<CurrIndex, T, std::tuple<Head, Tail...>, std::enable_if_t<std::is_same_v<Head, T>>> {
    static constexpr std::size_t value = CurrIndex;
};

template <class T, class TupleT>
constexpr std::size_t tuple_first_type_index_v = tuple_first_type_index<0, T, TupleT>::value;

//template <class ...T>
//static constexpr auto MergeSort(std::tuple<T...> tuple) {
//    static_assert (sizeof... (T), "Cannot sort empty tuple");
//    if constexpr(sizeof... (T) == 1) return tuple;
//    else {

//        auto [left, right] = SplitTupleIn2(tuple);
//        return MergeSortedLists(MergeSort(left), MergeSort(right));

//    }


//}
template<std::size_t FromI, std::size_t ToI, class TupleT, class Sequence, typename TTT=void >
struct tuple_types_interval;

template<std::size_t FromI, std::size_t ToI, class TupleT, std::size_t ...Is>
struct tuple_types_interval<FromI, ToI, TupleT, std::index_sequence<Is...>, std::enable_if_t<sizeof... (Is) < ToI-FromI>>
    : tuple_types_interval<FromI, ToI, TupleT, std::index_sequence<Is..., FromI+sizeof... (Is)>> {

};

// base case
template<std::size_t FromI, std::size_t ToI, class TupleT, std::size_t ...Is>
struct tuple_types_interval<FromI, ToI, TupleT, std::index_sequence<Is...>, std::enable_if_t<sizeof... (Is) == ToI-FromI>> {
    using type = std::tuple<std::tuple_element_t<Is,TupleT >...>;
};

template<std::size_t FromI, std::size_t ToI, class TupleT>
using tuple_types_interval_t= typename tuple_types_interval<FromI, ToI, TupleT, std::index_sequence<>>::type;

template<std::size_t Index, class TupleT>
struct tuple_type_extracter;

template<typename ... input_t>
using tuple_cat_t=
decltype(std::tuple_cat(
    std::declval<input_t>()...
));


template<std::size_t Index, class ...TupleTypes>
struct tuple_type_extracter<Index, std::tuple<TupleTypes...>> {
    using Tpl = std::tuple<TupleTypes...>;
    using ExtractedT = std::tuple_element_t<Index, Tpl>;
    using RestT = tuple_cat_t<tuple_types_interval_t<0, Index, Tpl>, tuple_types_interval_t<Index+1, sizeof... (TupleTypes), Tpl>>;

//    std::tuple<std::tuple_element_t<0, std::tuple<TupleTypes...>>, std::tuple_element_t<Index+1, std::tuple<TupleTypes...>>>;
};

#endif // UTILS_H
