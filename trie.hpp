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

template<std::size_t I, typename SString>
struct StringWithKey{
    static constexpr auto Index = I;
    using SST = SString;
    static constexpr char value = ss_char_getter_v<Index, SST>;
};

template <char Cv, typename SString>
using string_with_key = std::integral_constant<char, Cv>;


template <typename KeyTuple, typename DataTuple>
struct group_splitter{};

template<char K, class V> struct KeyedSet {
    static constexpr char  KeyChar = K;
    using Set = V;
};

template <class ...StringWithKey, typename ...SStringsWithKey>
struct group_splitter<std::tuple<StringWithKey...>,  std::tuple<SStringsWithKey...> >{
    using type = std::tuple<
            KeyedSet<
                StringWithKey::value,
                by_value_filterer_t<char, StringWithKey::value, std::tuple<SStringsWithKey...> >
            >
    ...>;
};

template <std::size_t I, typename LGroup>
struct LayerSplitter{};



template <std::size_t I, typename StringsWithKeysTuple, typename TTT=void >
struct TrieLayer{};


template<char K, class V> struct ChildLayerT {
    static constexpr char Key = K;
    using Layer = V;
};

template < std::size_t I, typename ...LayerKeyedStringTuples>
struct LayerSplitter<I, std::tuple<LayerKeyedStringTuples...>>{
    using type = std::tuple<
                ChildLayerT<
                    LayerKeyedStringTuples::KeyChar,
                    TrieLayer<I, typename  LayerKeyedStringTuples::Set>
                >
    ...>;
};

template <typename SS>struct SSMaxW{
    static constexpr auto value = SS::Size;
    using Member = SS;
};

template <typename KeyItems>
struct concrete_keys_extractor{};

template <typename ...StringWithKeyT>
struct concrete_keys_extractor<std::tuple<StringWithKeyT...>> {
    static constexpr std::array<char, sizeof...(StringWithKeyT)> Keys = {StringWithKeyT::value...};
};

//TODO skip equal Layers!!


template <typename InputTuple>
struct member_extractor{};

template <typename ...ItemT>
struct member_extractor<std::tuple<ItemT...>> {
    using type = std::tuple<typename ItemT::Member...>;
};
template <class T> using member_extractor_t = typename member_extractor<T>::type;

template <std::size_t I,  typename ... StringWithKeyT>
struct TrieLayer<I, std::tuple<StringWithKeyT...>,
        std::enable_if_t< (sizeof...(StringWithKeyT) > 1 && I < max_finder_v<std::tuple<SSMaxW<typename StringWithKeyT::SST> ...>>) >
        >
{

    using ThisStrings = member_extractor_t<sorted_tuple_t  <
            std::tuple<SSMaxW<typename StringWithKeyT::SST> ...>
        >>;
    using NodeString = typename std::tuple_element_t<0, ThisStrings>;
    static constexpr bool HasFullString = I >= NodeString::Size;

private:
    static constexpr auto MinLength = min_finder_v<std::tuple<SSMaxW<typename StringWithKeyT::SST> ...>>;

    using StringEndedHere = by_value_filterer_t<std::size_t, MinLength ,std::tuple<SSMaxW<typename StringWithKeyT::SST> ...> >;
    static constexpr std::size_t DepthLevel = I;
    using KeyedStrings = std::tuple<StringWithKey<I, typename StringWithKeyT::SST>...>;
    using keys = unique_only_getter_t< KeyedStrings>;
    static constexpr auto KeyChars = concrete_keys_extractor<keys>::Keys;
    using groups = typename group_splitter<keys,
                                                KeyedStrings
                                          >::type;
public:
    using NextNodes = typename LayerSplitter<I+1, groups>::type;
    static constexpr bool Last = std::tuple_size_v<NextNodes> == 0;
};

template <std::size_t I, typename ... StringWithKeyT>
struct TrieLayer<I, std::tuple<StringWithKeyT...>,
        std::enable_if_t< (sizeof...(StringWithKeyT) == 1 || I == max_finder_v<std::tuple<SSMaxW<typename StringWithKeyT::SST> ...>>) >
        >
{
    using ThisStrings = std::tuple<typename StringWithKeyT::SST...>;
    using NodeString = std::tuple_element_t<0, ThisStrings>;

private:
    static constexpr std::size_t DepthLevel = I;
    using KeyedStrings = std::tuple<StringWithKey<I, typename StringWithKeyT::SST>...>;
    using keys = unique_only_getter_t< KeyedStrings>;
    static constexpr auto KeyChars = concrete_keys_extractor<keys>::Keys;
    static_assert (sizeof... (StringWithKeyT) == 1, "Trie error: looks like there are identical strings in source tuple");
public:
    using NextNodes = std::tuple<>;
    static constexpr bool HasFullString = true;
    static constexpr bool Last = true;
};

template <typename Src,  typename V = void>
struct StaticTrie{};

template <typename ... Strings>
struct StaticTrie<std::tuple<Strings...>/*, std::enable_if_t<
        !(true && ... && SString::is_ss_v<Strings>)
        >*/>
{
    static_assert(true, "Duplicates in source string tuple are not allowed"); //TODO
    using L = TrieLayer<0, std::tuple<StringWithKey<0, Strings>...>>;

    void print() {

        auto iterator = []( auto & layer, const auto & self) -> void{
            using LT = typename std::remove_reference_t<decltype (layer)>::Layer;
//            cout << "Index: " << LT::DepthLevel << " ";

            if constexpr(LT::HasFullString) {
//                cout << "END: " << LT::NodeString::to_str();
            } else {
                for(auto ch: LT::KeyChars) {
//                    cout  << " " << ch << " " ;
                }
            }
//            cout    << endl;
            iterateTuple(typename LT::NextNodes(), self, self);
        };

        iterateTuple(L::NextNodes(), iterator, iterator);
    }

    template<bool IsLast, bool HasFull, std::size_t I, typename StringsT, typename NString>
    struct MatchRes{
        using MatchedStrings = StringsT;
        using SourceStrings = std::tuple<Strings...>;
        using NodeString = NString;
        static constexpr bool hasFull = HasFull;
        static constexpr bool isLast = IsLast;
        static constexpr std::size_t Index = I;
    };
    template<class Iter, class Clb>
    static Iter search (Iter iter, Iter end, Clb clb ) {
        bool to_continue = true;
        std::size_t index = 0;
        char symbol = *iter;
        int counter = 0;
        auto searcher= [&]( auto & layer, const auto & self) -> void{
            using Item =  std::remove_reference_t<decltype (layer)>;
            using LT = typename Item::Layer;
            constexpr char key = Item::Key;

            if(!to_continue || key != symbol) return;

            to_continue = clb(MatchRes<LT::Last, LT::HasFullString,
                                tuple_first_type_index_v<typename LT::NodeString, std::tuple<Strings...>>,
                                typename LT::ThisStrings, typename LT::NodeString>{}
                              );
            to_continue = ! LT::Last;
            if(to_continue) {
                ++iter;
                ++index;
                symbol = * iter;
                if (iter == end) to_continue = false;
                iterateTuple(typename LT::NextNodes(), self, self);
            }

            counter ++;

        };
        iterateTuple(typename L::NextNodes(), searcher, searcher);
        return iter;
    }
};

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
