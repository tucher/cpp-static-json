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
            TS("props"), Array<
                Bool,
                Int,
                Int,
                Object<
                    TS("flag1"), Bool,
                    TS("counter"), Int,
                    TS("msg"), String<10>,
                    TS("string"), Object<
                        N<TS("active"), Bool>,
                        N<TS("size"), Int>,
                        N<TS("name"), String<10>>
                    >
                >
                >
            ,
            TS("error"), String<10>
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

    obj.at<TS("error")>() = "something";
    obj.at<TS("props")>().at<1>() =  5000001;
    obj.at<TS("props")>().at<2>() = 42;
    obj.at<TS("props")>().at<3>().at<TS("string")>().at<TS("active")>() = -101452;
    obj.at<TS("props")>().at<3>().at<TS("string")>().at<TS("name")>() = "foo moo";
    obj.at<TS("props")>().at<3>().at<TS("string")>().at<TS("size")>() = 666;

    char output[Msg1Type::MaxStrSize+1];
    for(std::size_t i = 0; i < Msg1Type::MaxStrSize; i ++)output[i] = 'X';

    auto bg = Iter((char*)output, Msg1Type::MaxStrSize+1);
    auto end = Iter((char*)output, Msg1Type::MaxStrSize+1, Msg1Type::MaxStrSize+1);

    Serialiser s(obj);
    s.Serialise(bg);
    *bg = 0;
    output[Msg1Type::MaxStrSize] = 0;
    cout << "serialisation output" << endl << output << endl;

//    auto pass = [ &s]() {
//        for(int i = 0; i < 10000000; i ++) {
//            char output[Msg1Type::MaxStrSize+1];
//            s.Serialise(output);
//        }
//    };
//    auto us = measureTime(pass);

//    std::cout << "serialisation speed:" << endl << 10000000.0/(double(us)) << " op/us" << endl;
}

#endif

int test_trie(char *data, size_t size);

template <auto TS>
struct TestSS {
  using SST = decltype (TS);

};

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

    cout << "Msg1Type::MaxStrSize " << Msg1Type::MaxStrSize << endl;

     measureSlowSer();

    using Msg1Type =
            Object<
                TS("array"), Array<
                    Bool,
                    Int,
                    Int,
                    Object<
                        TS("boolean_field"), Bool,
                        TS("int_field"), Int>,
                        TS("string"), String<10>,
                        TS("string"), Object<
                            TS("boolean_field"), Bool,
                            TS("int_field"), Int,
                            TS("string"), String<10>
                        >
                    >
                ,
                TS("string"), String<10>
            >
           ;
    Msg1Type ();

    using Msg2Type =
            Object<
                TS("bf"), Bool,
                TS("obj"), Object<
                    TS("states"), Array<Bool, Bool, Bool>,
                    TS("flag"), Bool
                >,
                TS("active"), Bool,
                TS("child"), Object<
                    TS("count"), Int,
                    TS("temp"), Double,
                    TS("msg"), String<10>
                >,
                TS("too_many"), Array<Int, Int, Int>,
                TS("too_few"), Array<TypeCalc::repeater_t<5, Object<TS("t"), Bool>>>
              >
           ;

    using TrieT = tuple_element_t<1, Msg2Type::MemberTupleT>::MemberT::Trie;
    static_assert (TrieT::L::Last == false);
    static_assert (tuple_size_v<TrieT::L::InputStringTuple> == 2);
    static_assert (std::tuple_element_t<0, TrieT::L::NextNodes>::Key == 's');
    static_assert (std::tuple_element_t<1, TrieT::L::NextNodes>::Key == 'f');

//    static_assert (std::tuple_element_t<0, Trie::L::NextNodes>::Layer::Last == true);
//    static_assert (std::tuple_element_t<1, Trie::L::NextNodes>::Layer::Last == true);

//    static_assert (is_same_v<std::tuple_element_t<0, Trie::L::NextNodes>::Layer::NodeString::ItemT, TS("state")>);
//    static_assert (is_same_v<std::tuple_element_t<1, Trie::L::NextNodes>::Layer::NodeString::ItemT, TS("flag")>);

    Msg2Type obj;
    obj.at<TS("bf")>() = true;
    Msg2Type obj2;
    obj2.at<TS("bf")>() = true;
    cout << "obj == obj2: "<<( obj == obj2) << endl;
    static constexpr char d[] = R"JS(   {
            "no_field1": "ffuuu",
            "no_field2": true,
            "no_field3": false,
            "no_field4": 3.14,
            "no_field5": 3,
            "no_field6": null,
               "active"   :  true ,
            "no_field7": ["ffuuu", true, false, 3.14, 3, null],
               "obj"  :  {
                    "flag" : true  ,
                    "states":  [false, true, false]
               },
               "child": {
                    "count": 1234,
                    "msg":   "fuumuuuu",
                    "no_field8": {"active"   :  true , "obj"  :  {"flag" : true  , "states":  [false, true, false] }, "child": {"count": 1234,    "msg":   "fuumuuuu" ,          "temp":3.14},"bf":false},
                    "temp":3.14
               },
               "bf":false,
               "too_many": [10, 11,12, 3, 4, 5],
            "too_few": [{},{},{}]


        }
          )JS";
    Msg2Type checker;
    checker.at<TS("bf")>() = false;
    checker.at<TS("obj")>().at<TS("states")>().at<0>() = false;
    checker.at<TS("obj")>().at<TS("states")>().at<1>() = true;
    checker.at<TS("obj")>().at<TS("states")>().at<2>() = false;
    checker.at<TS("obj")>().at<TS("flag")>() = true;
    checker.at<TS("active")>() = true;
    checker.at<TS("child")>().at<TS("count")>() = 1234;
    checker.at<TS("child")>().at<TS("temp")>() = 3.14;
    checker.at<TS("child")>().at<TS("msg")>() = "fuumuuuu";
    checker.at<TS("too_many")>().at<0>() = 10;
    checker.at<TS("too_many")>().at<1>() = 11;
    checker.at<TS("too_many")>().at<2>() = 12;


    auto bg = Iter((char*)d, sizeof (d));
    auto end = Iter((char*)d, sizeof (d), sizeof (d));

    obj.Deserialise(bg, end);
    if( bg != end) {
        cout << "Deser success" << endl;
    } else {
        cout << "Deser error";
    }

    bool setFlag =  obj.at<TS("too_few")>().at<0>().wasSet &&
    obj.at<TS("too_few")>().at<1>().wasSet &&
    obj.at<TS("too_few")>().at<2>().wasSet &&
    !obj.at<TS("too_few")>().at<3>().wasSet &&
    !obj.at<TS("too_few")>().at<4>().wasSet ;
    if(obj == checker&& setFlag) {
        cout << "Deser correct" << endl;;
    } else {
        cout << "Deser INCORRECT" << endl;;
    }
    return 0;
#endif
}
