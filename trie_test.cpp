#include "trie.hpp"
#include "../comadapterdriver/utils/static_string.h"
#include "utils.h"
#include <iostream>

using namespace std;
using namespace SString;
static_assert (tuple_first_type_index_v<SS("34"), std::tuple<SS("123"), SS("54") , SS("34")>> == 2, "works");

using extr_test = tuple_type_extracter<2, std::tuple<int, float, char>>;
static_assert(is_same_v<extr_test::ExtractedT, char>, "dddd");
static_assert(is_same_v<extr_test::RestT, std::tuple<int, float>>, "dddd");


static_assert(is_same_v<std::tuple<float>, tuple_types_interval_t<1,2,  std::tuple<int, float, char, bool>>>, "dddd");
static_assert(is_same_v<std::tuple<float, char>, tuple_types_interval_t<1,3,  std::tuple<int, float, char, bool>>>, "dddd");
static_assert(is_same_v<std::tuple<float, char, bool>, tuple_types_interval_t<1,4,  std::tuple<int, float, char, bool>>>, "dddd");
static_assert(is_same_v<std::tuple<int, float>, tuple_types_interval_t<0,2,  std::tuple<int, float, char, bool>>>, "dddd");

static_assert(min_finder<std::tuple<integral_constant<int, 5>, integral_constant<int, 4>, integral_constant<int, 8>>>::value == 4, "dddd");

template<int V>
using C = integral_constant<int, V>;
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
static_assert(is_same_v<sorted_t,  std::tuple<
              C<2>,
              C<4>,
              C<4>,
              C<5>,
              C<5>,
              C<8>,
              C<8>
              >>, "Sorted not works");

using uniques = unique_only_getter_t<to_sort>;

static_assert(std::tuple_size_v<uniques> == 4);
static_assert(is_same_v<uniques,  std::tuple<
              C<8>,
              C<5>,
              C<4>,
              C<2>
              >>, "Distinct fail");

using filtered = by_value_filterer_t<int, 4, to_sort>;
static_assert(std::tuple_size_v<filtered> == 2);
static_assert(is_same_v<filtered,  std::tuple<
              C<4>,
              C<4>
              >>, "Filter fail");


using strings = std::tuple <
    SS("moscow"),
    SS("russia"),
    SS("volumetric"),
    SS("russian"),
    SS("ruble"),
    SS("volume"),
    SS("rude"),
    SS("rudiment"),
    SS("voltage"),
    SS("vodka"),
    SS("voda"),
    SS("vodokanal")
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

    int result = 0;
    auto clb = [&](std::size_t index, auto curIter,
                                      auto endIter,
                                      auto mathedStrTuple,
                                      auto nodeString,
                                      bool hasFulString,
                                      bool isLastNodeInChain) -> bool {
        result = index;
        using NodeString = decltype (nodeString);

        if(hasFulString) {

//            cout << index << "Full Node string: " << nodeString.to_str() << endl;

        }
        if(isLastNodeInChain) {
//            cout << " Success at index " << index << "!!!: " << nodeString.to_str() << endl;
//            result = 0;
            return false;
        }
//        cout << "Matched: " << endl;
//        iterateTuple(mathedStrTuple, [](auto &s){
//            cout << "\t " << s.to_str() << endl;
//        });
//        cout << endl;
        return true;
    };
    using trie = StaticTrie::trie_t<strings>;


    //Iter endedAt = trie::search(bg, end, clb);
    //cout << "Search ended at " << endedAt -bg << endl;

    trie::search(data, data + size, clb);
    return result;
}
