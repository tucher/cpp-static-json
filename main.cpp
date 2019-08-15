#include <iostream>
#include <tuple>
#include "../comadapterdriver/utils/static_string.h"
#include "../comadapterdriver/utils/utils.h"
#include <cstdio>
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

    using  SerialisedTemplate = filled_ss<' ', 5>;
    static constexpr std::size_t StrSize = SerialisedTemplate::Size;
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
    void SerialiseFast(Iter it) {
        Serialise(it);
    }
};

using Bool = BaseBool<bool>;

template <std::size_t MaxDigits, typename FPType>
class Double {
    FPType m_value = 0;
public:
    using  SerialisedTemplate = filled_ss<' ', MaxDigits + 2>;
    static constexpr std::size_t StrSize = SerialisedTemplate::Size;

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

    template <typename Iter>
    void SerialiseFast(Iter it) {
        Serialise(it);
//        char b[StrSize+1];
//        std::size_t r = std::snprintf(b, StrSize+1, "%g", m_value);
//        for(std::size_t i = 0; i < r; i ++) {
//            *it = b[i];
//            ++it;
//        }
    }
};

template <std::size_t LengthMax>
class String {
    std::array<char, LengthMax> m_value;
public:
    using  SerialisedTemplate = concat<SS("\""), filled_ss<' ', LengthMax+1>>;
    static constexpr std::size_t StrSize = SerialisedTemplate::Size;
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
    template <typename Iter>
    void SerialiseFast(Iter it) {
        Serialise(it);
//        std::size_t i = 0;
//        ++it;
//        for(; i < LengthMax && m_value[i] != 0; i++) {
//            *it = m_value[i];
//            ++it;
//        }
//        *it = '"';
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
    using  SerialisedTemplate = filled_ss<' ', MaxDigits + 1>;
    static constexpr std::size_t StrSize = SerialisedTemplate::Size;
    operator IntT&() {
        return m_value;
    }
    IntT & operator=(IntT v) {
        m_value = v;
        return *this;
    }
    template <typename Iter>
    void Serialise(Iter it) {
//        char b[StrSize+1];
//        std::size_t r = std::snprintf(b, StrSize+1, "%lld", std::int64_t(m_value));

//        for(std::size_t i = 0; i < r; i ++) {
//            *it = b[i];
//            ++it;
//        }
//        for(std::size_t i = r; i < StrSize; i++) {
//            *it = ' ';
//            ++it;
//        }
        auto f = it;
        for(std::size_t i = 0; i < StrSize; i++) {
                    *it = ' ';
                    ++it;
                }
        i64toa_naive(m_value, f);
    }
    template <typename Iter>
    void SerialiseFast(Iter it) {
        Serialise(it);
//        char b[StrSize+1];
//        std::size_t r = std::snprintf(b, StrSize+1, "%lld", std::int64_t(m_value));
//        for(std::size_t i = 0; i < r; i ++) {
//            *it = b[i];
//            ++it;
//        }

//        i64toa_naive(m_value, it);
    }
};


template <typename FirstMember = SS(""), typename ... OtherMembers>
struct ArrayConcatHelper {
    using SerialisedTemplate = concat<typename FirstMember::SerialisedTemplate, concat<SS(","), typename OtherMembers::SerialisedTemplate>...>;
};

//template <>
//struct ArrayConcatHelper <SS("")> {
//    using SerialisedTemplate = SS("");
//}

template <typename ...Members>
class Array {
    std::tuple<Members...> m_members;

public:
    static constexpr std::size_t  Length = sizeof... (Members);
    using  SerialisedTemplate = SString::concat<SS("["), typename ArrayConcatHelper<Members...>::SerialisedTemplate, SS("]")>;

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
    template <typename Iter>
   void SerialiseFast(Iter it) {
       ++it;
       foreach([&](auto &m) {
           m.SerialiseFast(it);
           it += m.StrSize+1;
       });
   }
};

template <typename SSName, typename Obj>
struct WithName{
    using Name = SSName;
    using MemberT = Obj;
    MemberT member;

    using  SerialisedTemplate = concat<SS("\"") , Name, SS("\":"), typename MemberT::SerialisedTemplate>;
    static constexpr std::size_t StrSize =
            + MemberT::StrSize // value
            + Name::Size //names
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
    void SerialiseFast(Iter it) {
        it += 2 + Name::Size + 1;
        member.SerialiseFast(it);
    }
};

template <typename ...Members>
class Object {
    std::tuple<Members...> m_members;
    static constexpr std::tuple<typename Members::Name...> m_names;

    template <typename> struct IsWithName : std::false_type { };
    template <typename SS, typename MemberT> struct IsWithName<WithName<SS, MemberT>> : std::true_type { };

    static_assert((true && ... && IsWithName<Members>::value), "Please, use WithName wrapper to create object members");
public:

    static constexpr std::size_t  Length = sizeof... (Members);
    static constexpr std::size_t StrSize = 2 + Length - 1 + (0 + ... + Members::StrSize);

    using  SerialisedTemplate = SString::concat<SS("{"), typename ArrayConcatHelper<Members...>::SerialisedTemplate, SS("}")>;

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
    void SerialiseFast(Iter it) {
        ++it;
        foreach([&](auto &m) {
            m.SerialiseFast(it);
            it += m.StrSize + 1;
        });
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
};


template <typename RootObj>
class FastSerialiser {
    RootObj &m_obj;

    template <int size, char... chars>
    constexpr std::array<char, size> ss_to_byte_array(X<tstring<chars...>>) {
        return std::array<char, size>{chars...};
    }

public:
    FastSerialiser(RootObj & obj):m_obj(obj) {}
    using  SerialisedTemplate = typename RootObj::SerialisedTemplate;
    static constexpr std::size_t StrSize = RootObj::StrSize;
    std::array<char, StrSize+1> output {ss_to_byte_array<StrSize+1>(typename RootObj::SerialisedTemplate())};

    void Serialise() {
        m_obj.SerialiseFast(output.data());
        output[StrSize] = 0;
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

void measureFastSer() {
    Msg1Type obj;


    obj.at<1>() = "something";
    obj.at<0>().at<1>() = 5000001;
    obj.at<0>().at<2>() = 42;
    obj.at<0>().at<3>().at<1>() = 101452;
    obj.at<0>().at<3>().at<2>() = "foo moo";

    FastSerialiser fs(obj);
    fs.Serialise();
    cout << "Fast ser output" << endl << fs.output.data() << endl;


    auto pass = [&obj, &fs]() {
        for(int i = 0; i < 10000000; i ++) {
            fs.Serialise();
        }
    };

    auto us = measureTime(pass);
    std::cout << "Fast serialisation speed:" << endl << 10000000.0/(double(us)) << " op/us" << endl;
}

void measureSlowSer() {
    Msg1Type obj;

    obj.at<1>() = "something";
    obj.at<0>().at<1>() =  5000001;
    obj.at<0>().at<2>() = 42;
    obj.at<0>().at<3>().at<1>() = 101452;
    obj.at<0>().at<3>().at<2>() = "foo moo";

    char output[Msg1Type::StrSize+1];
    for(int i = 0; i < Msg1Type::StrSize; i ++)output[i] = 'X';

    Serialiser s(obj);
    s.Serialise(output);
    output[Msg1Type::StrSize] = 0;
    cout << "Slow ser output" << endl << output << endl;

    auto pass = [&obj, &s]() {
        for(int i = 0; i < 10000000; i ++) {
            char output[Msg1Type::StrSize+1];
            s.Serialise(output);
        }
    };
    auto us = measureTime(pass);

    std::cout << "Slow serialisation speed:" << endl << 10000000.0/(double(us)) << " op/us" << endl;
}

int main()
{
    BoolLike boolObj = false;

    BaseBool<BoolLike> jsonBool(boolObj);
    jsonBool = true;
    if constexpr (is_same_v<decltype (boolObj), bool>) {
        cout << "boolObj is bool = " << boolObj << endl;
    }
    bool v = jsonBool;

    cout << "Msg1Type::StrSize " << Msg1Type::StrSize << endl;
    cout << "Msg1Type::SerialisedTemplate::to_str() " << endl << Msg1Type::SerialisedTemplate::to_str() << endl;




    measureFastSer();
     measureSlowSer();






    return 0;
}
