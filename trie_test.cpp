#include "trie.hpp"
#include "utils.h"
#include <iostream>

using namespace std;
using namespace SString;


using strings = std::tuple <
    SS("moscow"),
    SS("russia"),
    SS("volumetric"),
    SS("russian"),
    SS("pussian"),
    SS("ruble"),
    SS("volume"),
    SS("rude"),
    SS("rudiment"),
SS("punch"),
//    SS("voltage"),
//    SS("vodka"),
//    SS("voda"),
//    SS("vodokanal"),
//    SS("prickly"),
//    SS("increase"),
//    SS("doubt"),
//    SS("tease"),
//    SS("arrest"),
//    SS("trail"),
//    SS("rhyme"),
//    SS("mindless"),
//    SS("regret"),
//    SS("quack"),

//    SS("copy"),
//    SS("nosy"),
//    SS("doctor"),
//    SS("metal"),
//    SS("fade"),
//    SS("strange"),
//    SS("care"),
//    SS("bubble"),
//    SS("drawer"),
//    SS("cushion"),
//    SS("passenger"),
//    SS("knife"),
//    SS("humdrum"),
//    SS("basin"),
//    SS("spoil"),
//    SS("bells"),
//    SS("combative"),
//    SS("meaty"),
//    SS("envious"),
//    SS("screeching"),
//    SS("pull"),
//    SS("compete"),
//    SS("floor"),
//    SS("boot"),
//    SS("head"),
//    SS("sturdy"),
//    SS("pat"),
//    SS("nutty"),
//    SS("miss"),
//    SS("value"),
//    SS("aquatic"),
//    SS("plausible"),
//    SS("white"),
//    SS("quixotic"),
//    SS("sugar"),
//    SS("dazzling"),
//    SS("understood"),
//    SS("yarn"),
//    SS("yielding"),
//    SS("preserve"),

//    SS("bee"),
//    SS("mother"),
//    SS("cars"),
//    SS("tired"),
//    SS("separate"),
//    SS("naive"),
//    SS("baby"),
//    SS("gleaming"),
//    SS("zipper"),
//    SS("tame"),
//    SS("literate"),
//    SS("yell"),
//    SS("badge"),
//    SS("modern"),


//    SS("twig"),
//    SS("volleyball"),
//    SS("old"),
//    SS("plain"),
//    SS("refuse"),
//    SS("null"),

//    SS("grandiose"),
//    SS("aboard"),
//    SS("bath"),
//    SS("top"),
//    SS("mountain"),

//    SS("cycle"),
//    SS("wish"),
//    SS("train"),
//    SS("vacuous"),
//    SS("potato"),
//    SS("same"),
//    SS("river"),
//    SS("lunch"),
//    SS("obese"),
//    SS("rule"),
//    SS("nerve"),
//    SS("bed"),
//    SS("cave"),
//    SS("wicked"),
//    SS("queue"),
//    SS("enjoy"),
//    SS("charming"),
//    SS("five"),
//    SS("placid"),
//    SS("short"),
//    SS("jobless"),
//    SS("lopsided"),
    SS("match")
>;

using strings3 = std::tuple <
    SS("a"),
    SS("an")
>;

using strings2 = std::tuple <
    SS("aab"),
    SS("aac"),
    SS("b")
>;


int test_trie(char *data, size_t size) {

    int result = -1;
    auto clb = [&](auto matchInfo) -> bool {
        using MatchInfo = decltype (matchInfo);

        if constexpr(MatchInfo::hasFull) {
            cout << "Full Node string: " << MatchInfo::NodeString::to_str() << endl;
        }
        if constexpr(MatchInfo::isLast) {
            result = MatchInfo::Index;
            return false;
        }
        cout << "Matched: " << endl;
        iterateTuple(typename MatchInfo::MatchedStrings(), [](auto &s){
            cout << "\t " << s.to_str() << endl;
        });
        cout << endl;
        return true;
    };
    using Trie = StaticTrie::StaticTrie<strings>;


    //Iter endedAt = trie::search(bg, end, clb);
    //cout << "Search ended at " << endedAt -bg << endl;

    Trie::search(data, data + size, clb);
    return result;
}
