#ifndef TRIE_HPP
#define TRIE_HPP

#include "utils.h"

/////////////TRIE COMPOSITION


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

template <std::size_t I, typename Source, typename LGroup>
struct LayerSplitter{};



template <std::size_t I, typename StringsTuple, typename StringsWithKeysTuple, typename TTT=void >
struct TrieLayer{};


template<char K, class V> struct ChildLayerT {
    static constexpr char Key = K;
    using Layer = V;
};

template < std::size_t I, typename ...SString, typename ...LayerKeyedStringTuples>
struct LayerSplitter<I, std::tuple<SString...>, std::tuple<LayerKeyedStringTuples...>>{
    using type = std::tuple<
                ChildLayerT<
                    LayerKeyedStringTuples::KeyChar,
                    TrieLayer<I, std::tuple<SString...>, typename  LayerKeyedStringTuples::Set>
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


//TODO it still doesnt work for situation like:
/*
using strings3 = std::tuple <
    SS("a"), //node and parent at the same time!
    SS("an")
>;
*/

template <typename InputTuple>
struct member_extractor{};

template <typename ...ItemT>
struct member_extractor<std::tuple<ItemT...>> {
    using type = std::tuple<typename ItemT::Member...>;
};
template <class T> using member_extractor_t = typename member_extractor<T>::type;

template <std::size_t I, typename ...SString , typename ... StringWithKeyT>
struct TrieLayer<I, std::tuple<SString...>, std::tuple<StringWithKeyT...>,
        std::enable_if_t< (sizeof...(StringWithKeyT) > 1 && I < max_finder_v<std::tuple<SSMaxW<typename StringWithKeyT::SST> ...>>) >
        >{
    using ThisStrings = member_extractor_t<sorted_tuple_t  <
            std::tuple<SSMaxW<typename StringWithKeyT::SST> ...>
        >>;
    static constexpr auto MinLength = min_finder_v<std::tuple<SSMaxW<typename StringWithKeyT::SST> ...>>;

    using StringEndedHere = by_value_filterer_t<std::size_t, MinLength ,std::tuple<SSMaxW<typename StringWithKeyT::SST> ...> >;

    using NodeString = typename std::tuple_element_t<0, ThisStrings>;


    static constexpr bool End = I >= NodeString::Size;



    static constexpr std::size_t DepthLevel = I;
    using SourceStrings = std::tuple<SString...>;


    using KeyedStrings = std::tuple<StringWithKey<I, typename StringWithKeyT::SST>...>;

    using keys = unique_only_getter_t< KeyedStrings>;

    static constexpr auto KeyChars = concrete_keys_extractor<keys>::Keys;

    using groups = typename group_splitter<keys,
                                                KeyedStrings
                                          >::type;
    using childLayers = typename LayerSplitter<I+1, std::tuple<SString...>, groups>::type;
};

template <std::size_t I, typename ...SString , typename ... StringWithKeyT>
struct TrieLayer<I, std::tuple<SString...>, std::tuple<StringWithKeyT...>,
        std::enable_if_t< (sizeof...(StringWithKeyT) == 1 || I == max_finder_v<std::tuple<SSMaxW<typename StringWithKeyT::SST> ...>>) >
        >{
    static constexpr std::size_t DepthLevel = I;
    using SourceStrings = std::tuple<SString...>;
     using KeyedStrings = std::tuple<StringWithKey<I, typename StringWithKeyT::SST>...>;
    using keys = unique_only_getter_t< KeyedStrings>;
    using ThisStrings = std::tuple<typename StringWithKeyT::SST...>;

    static constexpr auto KeyChars = concrete_keys_extractor<keys>::Keys;

    static constexpr bool End = true;
    using childLayers = std::tuple<>;
    static_assert (sizeof... (StringWithKeyT) == 1, "Trie error, looks like there is identical strings in source tuple");
    using NodeString = std::tuple_element_t<0, ThisStrings>;
};

template <typename StringsTuple, typename Output>
struct trie_builder {};

template <typename ... Strings, typename ... OutputItems>
struct trie_builder<std::tuple<Strings...>, std::tuple<OutputItems...>>
{
    static_assert(std::is_same_v<bool, bool>, "Duplicates in source string tuple are not allowed");
    using L = TrieLayer<0, std::tuple<Strings...>, std::tuple<StringWithKey<0, Strings>...>>;

    void print() {

        auto iterator = []( auto & layer, const auto & self) -> void{
            using LT = typename std::remove_reference_t<decltype (layer)>::Layer;
//            cout << "Index: " << LT::DepthLevel << " ";

            if constexpr(LT::End) {
//                cout << "END: " << LT::NodeString::to_str();
            } else {
                for(auto ch: LT::KeyChars) {
//                    cout  << " " << ch << " " ;
                }
            }
//            cout    << endl;
            iterateTuple(typename LT::childLayers(), self, self);
        };

        iterateTuple(L::childLayers(), iterator, iterator);
    }


    template<class Iter, class Clb>
    static Iter search (Iter iter, Iter end, Clb clb ) {
        bool to_continue = true;
        std::size_t index = 0;
        auto searcher= [&index, &iter, &end, &clb, &to_continue]( auto & layer, const auto & self) -> void{
            if(!to_continue || iter == end) return;
            using Item =  std::remove_reference_t<decltype (layer)>;
            using LT = typename Item::Layer;
            constexpr char key = Item::Key;


            if(key == *iter) {
                to_continue = clb(index, iter, end, typename LT::ThisStrings(), typename LT::NodeString(), LT::End);
                if(to_continue) {
                    iter++;
                    index ++;
                    //                cout << "Going into node with strings: " << endl;
                    //                iterateTuple(typename LT::ThisStrings(), [](auto &s){
                    //                    cout << "\t " << s.to_str() << endl;
                    //                });
                    iterateTuple(typename LT::childLayers(), self, self);
                }
            }
        };

//        while(iter != end && to_continue) {
            iterateTuple(typename L::childLayers(), searcher, searcher);
//            if(iter == end) break;
//            iter ++;
//            index ++;
//        }
        return iter;
    }
};

template<class StringsTuple>
using trie_t = trie_builder<StringsTuple, std::tuple<>>;

#endif // TRIE_HPP
