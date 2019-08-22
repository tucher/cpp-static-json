#include "../template-tools/trie/trie.hpp"
#include "../template-tools/typestring/typestring.hpp"
#include "utils.h"
#include <iostream>

using namespace std;
using namespace TypeStringLiteralExploder;


using strings = std::tuple <
//    TS("moscow"),
//    TS("russia"),
//    TS("volumetric"),
//    TS("russian"),
//    TS("pussian"),
//    TS("ruble"),
//    TS("volume"),
//    TS("rude"),
//    TS("rudiment"),
//    TS("punch"),
//    TS("voltage"),
//    TS("vodka"),
//    TS("voda"),
//    TS("vodokanal"),
//    TS("prickly"),
//    TS("increase"),
//    TS("doubt"),
//    TS("tease"),
//    TS("arrest"),
//    TS("trail"),
//    TS("rhyme"),
//    TS("mindless"),
//    TS("regret"),
//    TS("quack"),
//    TS("copy"),
//    TS("nosy"),
//    TS("doctor"),
//    TS("metal"),
//    TS("fade"),
//    TS("strange"),
//    TS("care"),
//    TS("bubble"),
//    TS("drawer"),
//    TS("cushion"),
//    TS("passenger"),
//    TS("knife"),
//    TS("humdrum"),
//    TS("basin"),
//    TS("spoil"),
//    TS("bells"),
//    TS("combative"),
//    TS("meaty"),
//    TS("envious"),
//    TS("screeching"),
//    TS("pull"),
//    TS("compete"),
//    TS("floor"),
//    TS("boot"),
//    TS("head"),
//    TS("sturdy"),
//    TS("pat"),
//    TS("nutty"),
//    TS("miss"),
//    TS("value"),
//    TS("aquatic"),
//    TS("plausible"),
//    TS("white"),
//    TS("quixotic"),
//    TS("sugar"),
//    TS("dazzling"),
//    TS("understood"),
//    TS("yarn"),
//    TS("yielding"),
//    TS("preserve"),
//    TS("bee"),
//    TS("mother"),
//    TS("cars"),
//    TS("tired"),
//    TS("separate"),
//    TS("naive"),
//    TS("baby"),
//    TS("gleaming"),
//    TS("zipper"),
//    TS("tame"),
//    TS("literate"),
//    TS("yell"),
//    TS("badge"),
//    TS("modern"),
//    TS("twig"),
//    TS("volleyball"),
//    TS("old"),
//    TS("plain"),
//    TS("refuse"),
//    TS("null"),
//    TS("grandiose"),
//    TS("aboard"),
//    TS("bath"),
//    TS("top"),
//    TS("mountain"),
//    TS("cycle"),
//    TS("wish"),
//    TS("train"),
//    TS("vacuous"),
    TS("potato"),
    TS("same"),
    TS("river"),
    TS("lunch"),
    TS("obese"),
    TS("rule"),
    TS("nerve"),
    TS("bed"),
    TS("cave"),
    TS("wicked"),
    TS("queue"),
    TS("enjoy"),
    TS("charming"),
    TS("five"),
    TS("placid"),
    TS("short"),
    TS("jobless"),
    TS("lopsided"),
    TS("match")
>;

using strings3 = std::tuple <
    TS("a"),
    TS("an")
>;

using strings2 = std::tuple <
    TS("abdhq"),
    TS("abdhr"),
    TS("abdhr8"),
    TS("abdis"),
    TS("abdit"),
    TS("abeju"),
    TS("abejv"),
    TS("abekw"),
    TS("abekx"),
    TS("acfly"),
    TS("acflz"),
    TS("acflz5"),
    TS("acfm1"),
    TS("acfm2"),
    TS("acgn"),
    TS("acgo"),
    TS("acgp3"),
    TS("acgp4")
>;

using strings4 = std::tuple <
    TS("state"),
    TS("flag")

>;
using Trie = StaticTrie::Trie<strings4>;
static_assert (Trie::L::Last == false);
static_assert (std::tuple_element_t<0, Trie::L::NextNodes>::Key == 's');
static_assert (std::tuple_element_t<1, Trie::L::NextNodes>::Key == 'f');

static_assert (std::tuple_element_t<0, Trie::L::NextNodes>::Layer::Last == true);
static_assert (std::tuple_element_t<1, Trie::L::NextNodes>::Layer::Last == true);

static_assert (is_same_v<std::tuple_element_t<0, Trie::L::NextNodes>::Layer::NodeString::ItemT, TS("state")>);
static_assert (is_same_v<std::tuple_element_t<1, Trie::L::NextNodes>::Layer::NodeString::ItemT, TS("flag")>);


int test_trie(char *data, size_t size) {
    using Trie = StaticTrie::Trie<strings>;

    int result = -1;
    auto clb = [&](auto matchInfo) -> char {
        data ++;
        using MatchInfo = decltype (matchInfo);
//        cout << "Matched: " << endl;
//        iterateTuple(typename MatchInfo::MatchedStrings(), [](auto s){
//            cout << "\t " << decltype (s)::ItemT::to_str() << endl;
//        });
//        cout << endl;
//        if constexpr(matchInfo.hasFull) {
//            cout << "Full Node string: " << MatchInfo::NodeString::to_str() << endl;
//        }
        if constexpr(matchInfo.hasFull) {
            result = matchInfo.index;
            return -1;
        }
        return *data;
    };



    //Iter endedAt = trie::search(bg, end, clb);
    //cout << "Search ended at " << endedAt -bg << endl;

    Trie::search(*data, clb);
//    cout << "Search ended at pos " << r - data << " of data " << data << endl;
    return result;
}
