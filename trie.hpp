#ifndef TRIE_HPP
#define TRIE_HPP

#include "template_utils.hpp"
#include "../comadapterdriver/utils/static_string.h"

namespace StaticTrie {


template <std::size_t I, typename  SString, typename TTT=void >
struct ss_char_getter{};

template <std::size_t I, typename  SString>
struct ss_char_getter<I, SString, std::enable_if_t<I < SString::Size> >{
    static constexpr auto value = SString::template get<I>();
};

template <std::size_t I, typename  SString>
struct ss_char_getter<I, SString, std::enable_if_t<I >= SString::Size> >{
    static constexpr auto value = char(0);
};

template <std::size_t I, typename  SString>
constexpr auto ss_char_getter_v = ss_char_getter<I, SString>::value;

template <std::size_t CurrentIndex, typename KeyIndexedStrings, typename SourceIndexedStrings>
struct LayerSplitter{};

template <std::size_t I, typename IndexedStrings, typename TTT=void >
struct TrieLayer{};


template<char K, class V> struct ChildLayerT {
    static constexpr char Key = K;
    using Layer = V;
};

template <std::size_t I, char Key, typename AllIndexedStrings> struct split_helper{};
template <std::size_t I, char Key, typename ...AllIndexedStrings> struct split_helper<I, Key, std::tuple<AllIndexedStrings...>>{
    template <typename IndexedStr> struct Filter {
        static constexpr bool value = ss_char_getter_v<I, typename IndexedStr::ItemT> == Key;
    };
    using type = by_value_filterer_t<Filter, std::tuple<AllIndexedStrings...>>;
};

template < std::size_t I, typename ...KeyIndexedStrings, typename ...AllIndexedStrings>
struct LayerSplitter<I, std::tuple<KeyIndexedStrings...>, std::tuple<AllIndexedStrings...>>{


    using type = std::tuple<
    ChildLayerT<
    ss_char_getter_v<I, typename KeyIndexedStrings::ItemT>,
    TrieLayer<I+1,
    typename split_helper<I,
    ss_char_getter_v<I, typename KeyIndexedStrings::ItemT>,
    std::tuple<AllIndexedStrings...>
    >::type
    >
    >
    ...>;
};

template <typename T> struct SizeExtractor {static constexpr std::size_t value = T::ItemT::Size;};
template <typename S1, typename S2> struct StringComparator {
    static constexpr bool value = !(typename S1::ItemT() < typename S2::ItemT());
}; // TODO
template <typename S1, typename S2, std::size_t Index> struct CharAtIndexComparator {
    static constexpr bool value = ss_char_getter_v<Index, typename S1::ItemT> < ss_char_getter_v<Index, typename S2::ItemT>;
};

template <typename S1, typename S2> struct StringSizeComparator {
    static constexpr bool value = S1::ItemT::Size < S2::ItemT::Size;
};

template <std::size_t I,  typename ... StringWithIndex>
struct TrieLayer<I, std::tuple<StringWithIndex...>,
        std::enable_if_t< (sizeof...(StringWithIndex) > 1 && I < max_finder_t<std::tuple<StringWithIndex...>,StringSizeComparator >::ItemT::Size) >
>
{
    using InputStringTuple = std::tuple<StringWithIndex ...>;

    using ThisStrings = sorted_tuple_t  <
        InputStringTuple,
        StringComparator
    >;
    using NodeString = typename std::tuple_element_t<0, ThisStrings>;
    static constexpr bool HasFullString = I >= NodeString::ItemT::Size;

    private:
    template<typename S1, typename S2>
    using comparator = CharAtIndexComparator<S1, S2, I>;
    template<typename S>
    using char_extractor = ss_char_getter<I, typename S::ItemT>;
    using keys = unique_only_getter_t<sorted_tuple_t<InputStringTuple, comparator>, char_extractor>;

    public:
    using NextNodes = typename LayerSplitter<I, keys, InputStringTuple>::type;
    static constexpr bool Last = std::tuple_size_v<NextNodes> == 0;
};

template <std::size_t I, typename ... StringWithIndex>
struct TrieLayer<I, std::tuple<StringWithIndex...>,
        std::enable_if_t< (sizeof...(StringWithIndex) == 1 || I == max_finder_t<std::tuple<StringWithIndex...>,StringSizeComparator >::ItemT::Size) >
>
{
    using InputStringTuple = std::tuple<StringWithIndex ...>;
    using ThisStrings = InputStringTuple;
    using NodeString = std::tuple_element_t<0, ThisStrings>;

    static_assert (sizeof... (StringWithIndex) == 1, "Trie error: looks like there are identical strings in source tuple");
    using NextNodes = std::tuple<>;
    static constexpr bool HasFullString = true;
    static constexpr bool Last = true;
};

template <typename Src,  typename V = void>
struct StaticTrie{};

template <typename ... IndexedStrings>
struct StaticTrie<std::tuple<IndexedStrings...>/*, std::enable_if_t<
                !(true && ... && SString::is_ss_v<Strings>)
                >*/>
{
    static_assert(true, "Duplicates in source string tuple are not allowed"); //TODO
    using L = TrieLayer<0, std::tuple<IndexedStrings...>>;

    template<bool IsLast, bool HasFull, std::size_t I, typename StringsT, typename NString>
    struct MatchRes{
        using MatchedStrings = StringsT;
        using SourceStrings = std::tuple<IndexedStrings...>;
        using NodeString = NString;
        static constexpr bool hasFull = HasFull;
        static constexpr bool isLast = IsLast;
        static constexpr std::size_t index = I;
    };
    template<class Clb>
    static void search (char symbol, Clb clb ) {
        bool to_continue = true;
        auto searcher= [&]( auto & layer, const auto & self) -> void{
            using Item =  std::remove_reference_t<decltype (layer)>;
            using LT = typename Item::Layer;
            constexpr char key = Item::Key;

            if(!to_continue || key != symbol) return;

            symbol = clb(MatchRes<LT::Last, LT::HasFullString,
                              LT::NodeString::I,
                              typename LT::ThisStrings, typename LT::NodeString::ItemT>{}
                              );
            if(symbol==-1) to_continue = false;

            if constexpr(LT::Last) to_continue = false;
            if(to_continue) {
                iterateTuple(typename LT::NextNodes(), self, self);
            }
        };
        iterateTuple(typename L::NextNodes(), searcher, searcher);
    }
};

template <typename ...SStrings>
using Trie = StaticTrie<indexed_types_t<SStrings...>>;
//template <typename ... Strings>
//struct StaticTrie<std::tuple<Strings...>, std::enable_if_t<
//        (true && ... && SString::is_ss_v<Strings>)
//        >>: StaticTrie<indexed_types_t<Strings...>>{};

//template <typename ... Strings>
//struct StaticTrie<Strings..., std::enable_if_t<
//        (true && ... && SString::is_ss_v<Strings>)
//        > >: StaticTrie<indexed_types_t<Strings...>>{};

}
#endif // TRIE_HPP
