#ifndef STATIC_JSON_HPP
#define STATIC_JSON_HPP

#include <tuple>
#include "../comadapterdriver/utils/static_string.h"
#include "template_utils.hpp"
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
//        using OT = std::decay_t<OtherT>;
        bool namesCorrect =  (true && ... && ( compare_v<typename tuple_element_t<Is, typename OtherT::MemberTupleT>::Name, typename tuple_element_t<Is, MemberTupleT>::Name>));
        bool valsCorrect =  (true && ... && (std::get<Is>(other.m_members).member == std::get<Is>(m_members).member));
        return namesCorrect && valsCorrect;
    }


    constexpr static size_t MinKeyL = StaticMin(Members::Name::Size...);
    constexpr static size_t MaxKeyL = StaticMax(Members::Name::Size...);


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

#endif // STATIC_JSON_HPP