#include <iostream>
#include "static_json.hpp"
#include "utils.h"
#define FFUUU
#ifdef FFUUU
using namespace StaticJSON;

class BoolLike {
    bool m_val;
public:
    BoolLike(bool v = false): m_val(v) {
        cout << "BoolLike::BoolLike()" << endl;
    }
    BoolLike& operator=(bool other) {
        cout << "BoolLike::operator=()" << endl;
        m_val = other;
        return *this;
    }
    operator bool() {
        cout << "BoolLike::operator bool()" << endl;
        return m_val;
    }
};
using Msg1Type =
        Object<
            SS("props"), Array<
                Bool,
                Int,
                Int,
                Object<
                    SS("flag1"), Bool,
                    SS("counter"), Int,
                    SS("msg"), String<10>,
                    SS("string"), Object<
                        N<SS("active"), Bool>,
                        N<SS("size"), Int>,
                        N<SS("name"), String<10>>
                    >
                >
                >
            ,
            SS("error"), String<10>
          >
       ;

#include <chrono>
template<typename Callable>
std::int64_t measureTime(Callable &&function) {
        auto t1 = std::chrono::high_resolution_clock::now();
        function();
        auto t2 = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();

        return  duration;
}


void measureSlowSer() {
    Msg1Type obj;

    obj.at<1>() = "something";
    obj.at<0>().at<1>() =  5000001;
    obj.at<0>().at<2>() = 42;
    obj.at<0>().at<3>().at<1>() = 101452;
    obj.at<0>().at<3>().at<2>() = "foo moo";

    char output[Msg1Type::StrSize+1];
    for(std::size_t i = 0; i < Msg1Type::StrSize; i ++)output[i] = 'X';

    Serialiser s(obj);
    s.Serialise(output);
    output[Msg1Type::StrSize] = 0;
    cout << "serialisation output" << endl << output << endl;

    auto pass = [ &s]() {
        for(int i = 0; i < 10000000; i ++) {
            char output[Msg1Type::StrSize+1];
            s.Serialise(output);
        }
    };
    auto us = measureTime(pass);

    std::cout << "serialisation speed:" << endl << 10000000.0/(double(us)) << " op/us" << endl;
}

#endif

int test_trie(char *data, size_t size);

int main(int , char **argv) {
    std::size_t s = 0; while(argv[1][s] != 0) s ++;
//    return test_trie(argv[1], s);
#ifdef FFUUU
    BoolLike boolObj = false;

    BaseBool<BoolLike> jsonBool(boolObj);
    jsonBool = true;
    if constexpr (is_same_v<decltype (boolObj), bool>) {
        cout << "boolObj is bool = " << boolObj << endl;
    }
//    bool v = jsonBool;

    cout << "Msg1Type::StrSize " << Msg1Type::StrSize << endl;

//     measureSlowSer();

    using Msg1Type =
            Object<
                SS("array"), Array<
                    Bool,
                    Int,
                    Int,
                    Object<
                        SS("boolean_field"), Bool,
                        SS("int_field"), Int>,
                        SS("string"), String<10>,
                        SS("string"), Object<
                            SS("boolean_field"), Bool,
                            SS("int_field"), Int,
                            SS("string"), String<10>
                        >
                    >
                ,
                SS("string"), String<10>
            >
           ;
    Msg1Type ();

    using Msg2Type =
            Object<
                SS("bf"), Bool,
                SS("obj"), Object<
                    SS("states"), Array<Bool, Bool, Bool>,
                    SS("flag"), Bool
                >,
                SS("active"), Bool,
                SS("child"), Object<
                    SS("count"), Int,
                    SS("temp"), Double,
                    SS("msg"), String<10>
                >
              >
           ;

    using TrieT = tuple_element_t<1, Msg2Type::MemberTupleT>::MemberT::Trie;
    static_assert (TrieT::L::Last == false);
    static_assert (tuple_size_v<TrieT::L::InputStringTuple> == 2);
    static_assert (std::tuple_element_t<0, TrieT::L::NextNodes>::Key == 's');
    static_assert (std::tuple_element_t<1, TrieT::L::NextNodes>::Key == 'f');

//    static_assert (std::tuple_element_t<0, Trie::L::NextNodes>::Layer::Last == true);
//    static_assert (std::tuple_element_t<1, Trie::L::NextNodes>::Layer::Last == true);

//    static_assert (is_same_v<std::tuple_element_t<0, Trie::L::NextNodes>::Layer::NodeString::ItemT, SS("state")>);
//    static_assert (is_same_v<std::tuple_element_t<1, Trie::L::NextNodes>::Layer::NodeString::ItemT, SS("flag")>);

    Msg2Type obj;
    obj.at<0>() = true;
    Msg2Type obj2;
    obj2.at<0>() = true;
    cout << "obj == obj2: "<<( obj == obj2) << endl;
    static constexpr char d[] = R"JS(   {
               "active"   :  true ,
               "obj"  :  {
                    "flag" : true  ,
                    "states":  [false, true, false]
               },
               "child": {
                    "count": 1234,
                    "msg":   "fuumuuuu",
                    "temp":3.14
               },
               "bf":false}
          )JS";
    Msg2Type checker;
    checker.at<0>() = false;
    checker.at<1>().at<0>().at<0>() = false;
    checker.at<1>().at<0>().at<1>() = true;
    checker.at<1>().at<0>().at<2>() = false;
    checker.at<1>().at<1>() = true;
    checker.at<2>() = true;
    checker.at<3>().at<0>() = 1234;
    checker.at<3>().at<1>() = 3.14;
    checker.at<3>().at<2>() = "fuumuuuu";


    auto bg = Iter(d, sizeof (d)-1);
    auto end = Iter(d, sizeof (d)-1, sizeof (d)-1);
    if(auto it = obj.Deserialise(bg, end); it == end) {
        cout << "Deser success" << endl;
    } else {
        cout << "Deser error";
    }

    if(obj == checker) {
        cout << "Deser correct" << endl;;
    } else {
        cout << "Deser INCORRECT" << endl;;
    }
    return 0;
#endif
}
