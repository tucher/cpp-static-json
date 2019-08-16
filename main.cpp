#include <iostream>
#include <tuple>
#include "../comadapterdriver/utils/static_string.h"
#include "utils.h"
#include <cstdio>
#include  <cctype>
#include <cstdlib>
#include <array>
using namespace std;

namespace  StaticJSON {
using namespace SString;

template <typename TypeT, typename TypeLikeT, bool> struct Opt {
    static constexpr bool is_ref = false;
    TypeT v;
};

template <typename TypeT, typename TypeLikeT> struct Opt<TypeT, TypeLikeT, true> {
    static constexpr bool is_ref = true;
    TypeLikeT&v;
};

template<typename TargetT, typename TypeLikeT>
constexpr bool use_ref_v = std::is_reference_v<TypeLikeT> || (!std::is_reference_v<TypeLikeT> && !std::is_same_v<TargetT, TypeLikeT>);

template<typename TargetT, typename TypeLikeT>
using OptionalRef = Opt<TargetT, TypeLikeT, use_ref_v <TargetT, TypeLikeT>>;

template <typename Iter>
Iter skipWSUntil(Iter it, Iter end, char c) {
    while(it != end) {
        if(*it == c) return it;
        else if(!std::isspace(*it)) return end;
        ++it;
    }
    return end;
}

template <typename Iter>
Iter skipWS(Iter it, Iter end) {
    while(it != end && std::isspace(*it) ) {
        ++it;
    }
    return it;
}

template <typename Iter, typename Pred>
Iter skipWSUntil(Iter it, Iter end, Pred c) {
    while(it != end) {
        bool p = c(*it);
        if(p) return it;
        else if(!std::isspace(*it) && !p) return end;

        ++it;
    }
    return end;
}

template<typename BoolLikeT = bool>
class BaseBool {


//    template <bool B> struct ValType : Opt<B>
//    {
//        double y;
//    };
    using DT = OptionalRef<bool, BoolLikeT>;
    DT m_value;

public:

    template<typename U = BoolLikeT>
    BaseBool(bool v = false, typename std::enable_if_t<!use_ref_v<bool, U>>* = 0) : m_value{v} { }

    template<typename U = BoolLikeT>
    BaseBool(U & v, typename std::enable_if_t<use_ref_v<bool, U&>>* = 0) : m_value{v} { }

    static constexpr std::size_t StrSize = 5;
    operator BoolLikeT&() {
        return m_value.v;
    }
    operator bool() const {
        return m_value.v;
    }
    BaseBool & operator=(bool v) {
        m_value.v = v;
        return *this;
    }
    template <typename Iter>
    void Serialise(Iter it) {
        if(bool(m_value.v)) {
            *it = 't'; ++it;
            *it = 'r';++it;
            *it = 'u';++it;
            *it = 'e';++it;
            *it = ' ';
        } else {
            *it = 'f';++it;
            *it = 'a';++it;
            *it = 'l';++it;
            *it = 's';++it;
            *it = 'e';
        }
    }
    template <typename Iter>
    Iter Deserialise(Iter it, Iter end) {
        if(it = skipWSUntil(it, end, [](char c){return c == 'f' || c == 't';}); it == end) return end;
        char t[] = "rue";
        char f[] = "alse";
        char *cmp = 0;
        bool to_assign = false;
        if(*it == 't') {
            cmp = t;
            to_assign = true;
        } else {
            cmp =f;
        }
        ++it;
        for(; *cmp != 0 && it != end; cmp ++) {
            if(*it != *cmp) return end;
            ++it;
        }
        if(it == end) return end;
        if(*cmp != 0) return end;

        if(it = skipWS(it, end); it == end) return end;
        m_value.v = to_assign;

        return it;
    }
    template<typename T >
    bool operator==(const T & other) const {
        return other.m_value.v == m_value.v;
    }
};

using Bool = BaseBool<bool>;

template <std::size_t MaxDigits, typename FPType>
class Double {
    FPType m_value = 0;
public:
    static constexpr std::size_t StrSize = MaxDigits + 2;

    operator FPType&() {
        return m_value;
    }
    Double & operator=(FPType v) {
        m_value = v;
        return *this;
    }
    template <typename Iter>
    void Serialise(Iter it) {
        char b[StrSize+1];
        std::size_t r = std::snprintf(b, StrSize+1, "%g", m_value);

        for(std::size_t i = 0; i < r; i ++) {
            *it = b[i];
            ++it;
        }
        for(std::size_t i = r; i < StrSize; i++) {
            *it = ' ';
            ++it;
        }
    }
    template<typename T >
    bool operator==(const T & other) const {
        return other.m_value == m_value;
    }
    template <typename Iter>
    Iter Deserialise(Iter it, Iter end) {
        if(it = skipWS(it, end); it == end) return end;
        char buf[20];
        int l = 0;
        while(!std::isspace(*it)) {
            buf[l] = *it;
            ++it;
            ++l;
        }
        buf[l] = 0;

        char *ptr = 0;
        m_value = std::strtod(buf, &ptr);
        if(ptr == buf) {
            m_value = 0;
            return end;
        }
        it += ptr - buf;
        return skipWS(it, end);
    }
};

template <std::size_t LengthMax>
class String {
    std::array<char, LengthMax> m_value;
public:
    static constexpr std::size_t StrSize = LengthMax+2;
    String(const char * v = "") {
        *this = v;
    }
    operator auto&() {
        return m_value;
    }
    String & operator=(const char * v) {
        std::size_t i = 0;
        for(; i < LengthMax && v[i] != 0; i ++) {
            m_value[i] = v[i];
        }
        if(i < LengthMax) {
            m_value[i] = 0;
        }
        return *this;
    }
    template <typename Iter>
    void Serialise(Iter it) {
        std::size_t i = 0;
        *it = '"';
        ++it;
        for(; i < LengthMax && m_value[i] != 0; i++) {
            *it = m_value[i];
            ++it;
        }
        *it = '"';
        ++it;
        for(std::size_t j = i; j < LengthMax; j ++) {
            *it = ' ';
            ++it;
        }
    }
    template<typename T >
    bool operator==(const T & other) const {
        return other.m_value == m_value;
    }

    template <typename Iter>
    Iter Deserialise(Iter it, Iter end) {
        if(it = skipWSUntil(it, end, '"'); it == end) return end;
        ++it;
        std::size_t i = 0;
        while(*it != '"' && it != end) {
            m_value[i] = *it;
            ++it;
            i++;
            if (i >= LengthMax) return end;
        }
        if(it == end) return end;
        if (i < LengthMax - 1) m_value[i] = 0;
        ++it;
        return it;
    }
};

template <std::size_t MaxDigits, typename IntT>
class Int {
    IntT m_value = 0;

    void u64toa_naive(uint64_t value, char* buffer) {
        char temp[StrSize+1];
        char *p = temp;
        do {
            *p++ = char(value % 10) + '0';
            value /= 10;
        } while (value > 0);

        do {
            *buffer++ = *--p;
        } while (p != temp);

//        *buffer = '\0';
    }

    void i64toa_naive(int64_t value, char* buffer) {

        uint64_t u = static_cast<uint64_t>(value);
        if (value < 0) {
            *buffer++ = '-';
            u = ~u + 1;
        }
        u64toa_naive(u, buffer);
    }

public:
    static constexpr std::size_t StrSize = MaxDigits + 1;
    operator IntT&() {
        return m_value;
    }
    IntT & operator=(IntT v) {
        m_value = v;
        return *this;
    }
    template <typename Iter>
    void Serialise(Iter it) {
        auto f = it;
        for(std::size_t i = 0; i < StrSize; i++) {
                    *it = ' ';
                    ++it;
                }
        i64toa_naive(m_value, f);
    }
    template<typename T >
    bool operator==(const T & other) const {
        return other.m_value == m_value;
    }

    template <typename Iter>
    Iter Deserialise(Iter it, Iter end) {
        if(it = skipWS(it, end); it == end) return end;
        char buf[20];
        int l = 0;
        while(!std::isspace(*it)) {
            buf[l] = *it;
            ++it;
            ++l;
        }
        buf[l] = 0;

        char *ptr = 0;
        m_value = std::strtoll(buf, &ptr, 10);
        if(ptr == buf) {
            m_value = 0;
            return end;
        }
        it += ptr - buf;
        return skipWS(it, end);
    }
};

template <typename ...Members>
class Array {
    std::tuple<Members...> m_members;

    template<typename OtherT, std::size_t ... Is>
    bool cmp_helper(const OtherT &other, std::index_sequence<Is...>) const {
        return (true && ... && (std::get<Is>(other.m_members) == std::get<Is>(m_members)));
    }
public:
    static constexpr std::size_t  Length = sizeof... (Members);

    static constexpr std::size_t StrSize = 2 + Length - 1 + (0 + ... + Members::StrSize);
    template<std::size_t index>
    auto & at() {
        static_assert (index < Length, "Out of range");
        return std::get<index>(m_members);
    }
    template<typename Callable>
    void foreach(Callable &&c) {
        iterateTuple(m_members, c);
    }
     template <typename Iter>
    void Serialise(Iter it) {
        *it = '[';
        ++it;
        if constexpr (Length > 0) {
            bool first = true;
            at<0>().Serialise(it);
            it += at<0>().StrSize;
            foreach([&](auto &m) {
                if(first) {
                    first = false;
                    return;
                }
                *it = ',';
                ++it;
                m.Serialise(it);
                it += m.StrSize;
            });
        }
       *it  = ']';
    }
    template <typename OtherT>
    bool operator==(const OtherT & other) const {
        return cmp_helper(other, std::make_index_sequence<Length>());
    }

    template <typename Iter>
    Iter Deserialise(Iter it, Iter end) {
        it = skipWSUntil(it, end, '[');
        if(it == end) return end;
         ++it;
        std::size_t filled = 0;
        bool err = false;

        foreach(
                [&](auto &v){
                if(err || *it == ']') return ;
                 Iter cur =  v.Deserialise(it, end);
                if(cur != end) {
                    filled ++;
                    while(cur != end && *cur != ']' && *cur != ',') ++cur;
                    if(cur == end) {err = true; return;}
                    if(*cur == ',') ++cur;

                    it = cur;
                    it = skipWS(it, end);
                    if(it == end) {err = true; return;}
                } else {
                    err = true;
                }
        });

        if (filled < Length) {
            //TODO mark other fields as uninit
        }
        if(err) return end;

        if(*it != ']' ) return end;
        if(it == end) return end;
        ++it;
        it = skipWS(it, end);
        return it;
    }
};

template <typename SSName, typename Obj>
struct WithName{
    using Name = SSName;
    using MemberT = Obj;
    MemberT member;

    static constexpr std::size_t StrSize =
            + MemberT::StrSize // value
            + Name::Size //name
            + ( 1 + 2) // colon and quates
            ;
    template <typename Iter>
    void Serialise(Iter it) {
        *it = '"';
        ++it;
        for(std::size_t i = 0; i < Name::Size; i ++) {
            *it = Name::to_str()[i];
            ++it;
        }
        *it = '"';
        ++it;
        *it = ':';
        ++it;
        member.Serialise(it);
    }
    template <typename Iter>
    Iter Deserialise(Iter it, Iter end) {
        it = skipWSUntil(it, end, '"');
        if(it == end) return end;

        ++it;
        for(std::size_t i = 0; i < Name::Size && it != end; i++) {
            if(Name::to_str()[i] != *it) {
                return end;
            }
            ++it;
        }
        if(it == end) return end;
        it = skipWSUntil(it, end, '"');
        if(it == end) return end;
        ++it;
        it = skipWSUntil(it, end, ':');
        if(it == end) return end;
        ++it;
        it = skipWS(it, end);
        if(it == end) return end;
        return member.Deserialise(it, end);
    }

    template <typename OtherT>
    bool operator==(const OtherT & other)const  {
        return compare_v<Name, OtherT::Name> && member == other;
    }
};

template <typename ...Members>
class Object {
    using MemberTupleT = std::tuple<Members...>;
    MemberTupleT m_members;


    template <typename> struct IsWithName : std::false_type { };
    template <typename SS, typename MemberT> struct IsWithName<WithName<SS, MemberT>> : std::true_type { };

    static_assert((true && ... && IsWithName<Members>::value), "Please, use WithName wrapper to create object members");

    template<typename OtherT, std::size_t ... Is>
    bool cmp_helper(const OtherT &other, std::index_sequence<Is...>) const {
        using OT = std::decay_t<OtherT>;
        bool namesCorrect =  (true && ... && ( compare_v<typename tuple_element_t<Is, typename OtherT::MemberTupleT>::Name, typename tuple_element_t<Is, MemberTupleT>::Name>));
        bool valsCorrect =  (true && ... && (std::get<Is>(other.m_members).member == std::get<Is>(m_members).member));
        return namesCorrect && valsCorrect;
    }


    constexpr static size_t MinKeyL = StaticMin(Members::Name::Size...);
    constexpr static size_t MaxKeyL = StaticMax(Members::Name::Size...);

    static_assert (IsInList(1, 1, 2, 3, 4), "IsInList");

    constexpr static bool finder() {


        return true;
    }
    constexpr static bool map = finder();
public:

    static constexpr std::size_t  Length = sizeof... (Members);
    static constexpr std::size_t StrSize = 2 + Length - 1 + (0 + ... + Members::StrSize);

    template<std::size_t index>
    auto & at() {
        static_assert (index < Length, "Out of range");
        return std::get<index>(m_members).member;
    }
    template<typename Callable>
    void foreach(Callable &&c) {
        iterateTuple(m_members, c);
    }

    template <typename Iter>
    void Serialise(Iter it) {
        *it = '{';
        ++it;
        if constexpr (Length > 0) {
            bool first = true;
            std::get<0>(m_members).Serialise(it);
            it += std::get<0>(m_members).StrSize;
            foreach([&](auto &m) {
                if(first) {
                    first = false;
                    return;
                }
                *it = ',';
                ++it;
                m.Serialise(it);
                it += m.StrSize;
            });
        }
       *it  = '}';
    }
    template <typename Iter>
    Iter Deserialise(Iter it, Iter end) {
        it = skipWSUntil(it, end, '{');
        if(it == end) return end;
         ++it;
        //props finding
        bool err = false;
        bool nothingFound = true;
        do {
            nothingFound = true;
            foreach(
                    [&](auto &kv){
                    if(err || !nothingFound || *it == '}') return ;
                     Iter cur =  kv.Deserialise(it, end);
                    if(cur != end) {
                        nothingFound = false;
                        it = cur;
                        it = skipWSUntil(it, end, [](char c){return c == '}'||c==',';});
                        if(cur == end) {err = true; return;}
                        if(*cur == ',') ++cur;
                        it = cur;
                        it = skipWS(it, end);
                        if(cur == end) {err = true; return;}
                    }
            });

            if(err) return end;
        } while(!nothingFound);
        it = skipWS(it, end);
        if(it == end) return end;
        if(*it != '}') return end;
        ++it;
        it = skipWS(it, end);
        return it;
    }
    template <typename OtherT>
    bool operator==(const OtherT & other) const {
        return cmp_helper(other, std::make_index_sequence<Length>());
    }
};

template <typename RootObj>
class Serialiser {
    RootObj &m_obj;

public:
    Serialiser(RootObj & obj):m_obj(obj) {}
    static constexpr std::size_t StrSize = RootObj::StrSize;

    template<typename Iter>
    void Serialise(Iter it) {
        m_obj.Serialise(it);
    }
};
}

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
            WithName<SS("array"), Array<
                Bool,
                Int<10, std::int32_t>,
                Int<5, std::int16_t>,
                Object<
                    WithName<SS("boolean_field"), Bool>,
                    WithName<SS("int_field"), Int<10, std::int32_t>>,
                    WithName<SS("string"), String<10>>,
                    WithName<SS("string"), Object<
                        WithName<SS("boolean_field"), Bool>,
                        WithName<SS("int_field"), Int<10, std::int32_t>>,
                        WithName<SS("string"), String<10>>
                    >>
                >
                >
            > ,
            WithName<SS("string"), String<10>>
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

static_assert (tuple_first_type_index_v<SS("34"), std::tuple<SS("123"), SS("54") , SS("34")>> == 2, "works");

using extr_test = tuple_type_extracter<2, std::tuple<int, float, char>>;
static_assert(is_same_v<extr_test::ExtractedT, char>, "dddd");
static_assert(is_same_v<extr_test::RestT, std::tuple<int, float>>, "dddd");


static_assert(is_same_v<std::tuple<float>, tuple_types_interval_t<1,2,  std::tuple<int, float, char, bool>>>, "dddd");
static_assert(is_same_v<std::tuple<float, char>, tuple_types_interval_t<1,3,  std::tuple<int, float, char, bool>>>, "dddd");
static_assert(is_same_v<std::tuple<float, char, bool>, tuple_types_interval_t<1,4,  std::tuple<int, float, char, bool>>>, "dddd");
static_assert(is_same_v<std::tuple<int, float>, tuple_types_interval_t<0,2,  std::tuple<int, float, char, bool>>>, "dddd");


int main()
{
//    constexpr auto sorted = MergeSort(make_tuple(1, 3, 2));

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
                WithName<SS("array"), Array<
                    Bool,
                    Int<10, std::int32_t>,
                    Int<5, std::int16_t>,
                    Object<
                        WithName<SS("boolean_field"), Bool>,
                        WithName<SS("int_field"), Int<10, std::int32_t>>,
                        WithName<SS("string"), String<10>>,
                        WithName<SS("string"), Object<
                            WithName<SS("boolean_field"), Bool>,
                            WithName<SS("int_field"), Int<10, std::int32_t>>,
                            WithName<SS("string"), String<10>>
                        >>
                    >
                    >
                > ,
                WithName<SS("string"), String<10>>
              >
           ;
    using Msg2Type =
            Object<
                WithName<SS("bf"), Bool>,
                WithName<SS("obj"), Object<
                    WithName<SS("arr"), Array<Bool, Bool, Bool>>,
                    WithName<SS("boolean_field"), Bool>
                >>,
                WithName<SS("bool"), Bool>,
                WithName<SS("obj_more"), Object<
                    WithName<SS("int"), Int<10, std::int32_t>>,
                    WithName<SS("float"), Double<10, float>>,
                    WithName<SS("string"), String<10>>
                >>
              >
           ;
    Msg2Type obj;
    obj.at<0>() = true;
    Msg2Type obj2;
    obj2.at<0>() = true;
    cout << "obj == obj2: "<<( obj == obj2) << endl;
    static constexpr char d[] = R"JS(   {
               "bool"   :  true ,
               "obj"  :  {
                    "boolean_field" : true  ,
                    "arr":  [false, true, false]
               },
               "obj_more": {
                    "int": 1234,
                    "string":   "fuumuuuu",
                    "float":3.14
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

    class Iter {
        const char *m_d = 0;
        std::size_t m_size = 0;
        std::size_t pos = 0;
    public:

        Iter(const char *d, std::size_t size, std::size_t offs = 0):m_d(d), m_size(size), pos(offs) {

        }
        void operator++() {
            pos++;
        }
        Iter& operator+=(std::size_t offs) {
            pos+=offs;
            return *this;
        }
        const char & operator*() {
            cout << "Read at " << pos << endl;
            return *( m_d + pos);
        }
        bool operator==(Iter other) {return other.m_d==d&&other.pos == pos;}
        bool operator!=(Iter other) {return other.m_d!=d||other.pos != pos;}
    };
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
}
