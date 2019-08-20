#ifndef STATIC_JSON_HPP
#define STATIC_JSON_HPP

#include <tuple>
#include "../comadapterdriver/utils/static_string.h"
#include "template_utils.hpp"
#include <cstdio>
#include  <cctype>
#include <cstdlib>
#include <array>
#include <trie.hpp>
#include <iostream>

using namespace std;

namespace  StaticJSON {

static constexpr std::size_t SkipNestedStructuresLimit = 20;


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
inline void skipWSUntil(Iter & it, Iter end, char c) {
    while(it != end) {
        char ch = *it;
        if(ch == c) return;
        else if(!std::isspace(ch)) {it = end; return;}
        ++it;
    }
}

template <typename Iter>
inline void skipWS(Iter & it, Iter end) {
    while(it != end && std::isspace(*it) ) {
        ++it;
    }
}

template <typename Iter, typename Pred>
inline void skipWSUntil(Iter & it, Iter end, Pred c) {
    while(it != end) {
        bool p = c(*it);
        if(p) return;
        else if(!std::isspace(*it) && !p) {it = end; return;}

        ++it;
    }
}

template <typename Iter>
void skipJSON(Iter& it, Iter end, std::size_t currentDepth);

template <typename Iter>
void skipObject(Iter& it, Iter end, std::size_t currentDepth) {
    if(*it != '{'){it = end; return;}
    currentDepth ++;
    if(currentDepth > SkipNestedStructuresLimit) {it = end; return;}
    ++it;
    skipWS(it, end);
    if(it == end) return;

    while(*it != '}' && it != end) {
        skipWSUntil(it, end, '"');
        if(it == end) return;
        ++it;
        while(it != end && *it != '"') {
            if(*it == '\\') {it = end; return;}
            if(!std::isprint(*it)) {it = end; return;}
            ++it;
        }
        if(it == end) { return;}
        ++it;
        if(it == end) {return;}
        skipWSUntil(it, end, ':');
        if(it == end) return;
        ++it;
        if(it == end) return;
        skipWS(it, end);
        if(it == end) return;

        skipJSON(it, end, currentDepth);
        skipWS(it, end);
        if(it == end) return;
        if(*it == ',') {
            ++it;
            if(it == end) return;
            skipWS(it, end);
            if(it == end) return;
        } else if(*it != '}')  {
            it = end; return;
        }
    }
    if(it == end) return;
    ++it;
    if(it == end) return;
    skipWS(it, end);
}


template <typename Iter>
void skipArray(Iter& it, Iter end, std::size_t currentDepth) {
    if(*it != '['){it = end; return;}
    currentDepth ++;
    if(currentDepth > SkipNestedStructuresLimit) {it = end; return;}
    ++it;
    skipWS(it, end);
    if(it == end) return;

    while(*it != ']' && it != end) {
        skipJSON(it, end, currentDepth);
        skipWS(it, end);
        if(it == end) return;
        if(*it == ',') {
            ++it;
            if(it == end) return;
            skipWS(it, end);
            if(it == end) return;
        } else if(*it != ']')  {
            it = end; return;
        }
    }
    if(it == end) return;
    ++it;
    if(it == end) return;
    skipWS(it, end);
}


template <typename Iter>
void skipJSON(Iter& it, Iter end, std::size_t currentDepth) {
    if (it == end) return;
    char c = *it;
    switch(c) {
    case '{':
    {
        skipObject(it, end, currentDepth);
        break;
    }
    case '[':
    {
        skipArray(it, end, currentDepth);
        break;
    }
    case 't':
    {
        ++it;
        if(it != end && *it != 'r') {it = end; return;}
        ++it;
        if(it != end && *it != 'u') {it = end; return;}
        ++it;
        if(it != end && *it != 'e') {it = end; return;}
        ++it;
        break;
    }
    case 'f':
    {
        ++it;
        if(it != end && *it != 'a') {it = end; return;}
        ++it;
        if(it != end && *it != 'l') {it = end; return;}
        ++it;
        if(it != end && *it != 's') {it = end; return;}
        ++it;
        if(it != end && *it != 'e') {it = end; return;}
        ++it;
        break;
    }
    case 'n':
    {
        ++it;
        if(it != end && *it != 'u') {it = end; return;}
        ++it;
        if(it != end && *it != 'l') {it = end; return;}
        ++it;
        if(it != end && *it != 'l') {it = end; return;}
        ++it;
        break;
    }
    case '-':
    case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
    {
        if(*it == '-') {
            ++it;
            if(it == end) return;
        }
        if(!(*it >= '0'&& *it <= '9'))  {it = end; return;}
        ++it;
        bool hasDot = false;
        while(it != end && !(isspace(*it) || *it == '}' || *it == ']' || *it == ',')) {
            if(!(*it >= '0'&& *it <= '9') && *it != '.')  {it = end; return;}
            if (*it == '.') {
                if(hasDot) {it = end; return;}
                hasDot = true;
            }
            ++it;
        }
        break;
    }
    case '"':
    {
        ++it;
        while(it != end && *it != '"') {
            if(*it == '\\') {it = end; return;}
            if(!std::isprint(*it)) {it = end; return;}
            ++it;
        }
        if(it != end && *it == '"') ++it;
        break;
    }
    default:
        it = end;
        ;
    }
    skipWS(it, end);
}

template<typename BoolLikeT = bool>
class BaseBool {
    using DT = OptionalRef<bool, BoolLikeT>;
    DT m_value;

public:
    bool wasSet = false;
    void ClearSetFlag() {wasSet = false;}
    template<typename U = BoolLikeT>
    BaseBool(bool v = false, typename std::enable_if_t<!use_ref_v<bool, U>>* = 0) : m_value{v} { }

    template<typename U = BoolLikeT>
    BaseBool(U & v, typename std::enable_if_t<use_ref_v<bool, U&>>* = 0) : m_value{v} { }

    static constexpr std::size_t MaxStrSize = 5;
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
    void Serialise(Iter & it) {
        if(bool(m_value.v)) {
            *it = 't';++it;
            *it = 'r';++it;
            *it = 'u';++it;
            *it = 'e';++it;
        } else {
            *it = 'f';++it;
            *it = 'a';++it;
            *it = 'l';++it;
            *it = 's';++it;
            *it = 'e';++it;
        }
    }
    template <typename Iter>
    void Deserialise(Iter & it, Iter end) {
        skipWSUntil(it, end, [](char c){return c == 'f' || c == 't';});
        if(it == end) return;
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
            if(*it != *cmp)  {it = end; return;}
            ++it;
        }
        if(it == end) return;
        if(*cmp != 0)  {it = end; return;}

        skipWS(it, end);
        if( it == end) return;
        m_value.v = to_assign;
        wasSet = true;
    }
    template<typename T >
    bool operator==(const T & other) const {
        return other.m_value.v == m_value.v;
    }
};

using Bool = BaseBool<bool>;

template <std::size_t MaxDigits, typename FPType>
class BaseDouble {
    FPType m_value = 0;
public:
    bool wasSet = false;
    void ClearSetFlag() {wasSet = false;}
    static constexpr std::size_t MaxStrSize = MaxDigits + 2;

    operator FPType&() {
        return m_value;
    }
    BaseDouble & operator=(FPType v) {
        m_value = v;
        return *this;
    }
    template <typename Iter>
    void Serialise(Iter & it) {
        char b[MaxStrSize+1];
        std::size_t r = std::snprintf(b, MaxStrSize+1, "%g", m_value);

        for(std::size_t i = 0; i < r && i < MaxStrSize; i ++) {
            *it = b[i];
            ++it;
        }

    }
    template<typename T >
    bool operator==(const T & other) const {
        return other.m_value == m_value;
    }
    template <typename Iter>
    void Deserialise(Iter & it, Iter end) {
        skipWS(it, end);
        if(it == end) return;
        char buf[20];
        int l = 0;
        char s = *it;
        while(!std::isspace(s) && s != '}' && s != ']' && s != ','&&it != end) {
            buf[l] = *it;
            ++it;
            ++l;
            s = *it;
        }
        if(it == end) return;
        buf[l] = 0;

        char *ptr = 0;
        m_value = std::strtod(buf, &ptr);
        if(ptr == buf) {
            m_value = 0;
             {it = end; return;}
        }
        skipWS(it, end);
        wasSet = true;
    }
};
using Double = BaseDouble<12, double>;


template <std::size_t LengthMax>
class String {
    std::array<char, LengthMax> m_value;
public:
    bool wasSet = false;
    void ClearSetFlag() {wasSet = false;}

    static constexpr std::size_t MaxStrSize = LengthMax+2;
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
    void Serialise(Iter & it) {
        *it = '"';
        ++it;
        for(std::size_t i = 0; i < LengthMax && m_value[i] != 0; i++) {
            *it = m_value[i];
            ++it;
        }
        *it = '"';
        ++it;
    }
    template<typename T >
    bool operator==(const T & other) const {
        return other.m_value == m_value;
    }

    template <typename Iter>
    void Deserialise(Iter& it, Iter end) {
        skipWSUntil(it, end, '"');
        if(it == end) return;
        ++it;
        std::size_t i = 0;
        while(*it != '"' && it != end) {
            m_value[i] = *it;
            ++it;
            i++;
            if (i >= LengthMax) it =  end;
        }
        if(it == end) return;
        if (i < LengthMax - 1) m_value[i] = 0;
        ++it;
        wasSet = true;
    }
};

template <std::size_t MaxDigits, typename IntT>
class BaseInt {
    IntT m_value = 0;

public:
    bool wasSet = false;
    void ClearSetFlag() {wasSet = false;}

    static constexpr std::size_t MaxStrSize = MaxDigits + 1;
    operator IntT&() {
        return m_value;
    }
    IntT & operator=(IntT v) {
        m_value = v;
        return *this;
    }
    template <typename Iter>
    void Serialise(Iter & it) {
        uint64_t u = static_cast<uint64_t>(m_value);
        if (m_value < 0) {
            *it = '-';
            ++it;
            u = ~u + 1;
        }
        char temp[MaxStrSize+1];
        char *p = temp;
        do {
            *p++ = char(u % 10) + '0';
            u /= 10;
        } while (u > 0);

        do {
            *it = *--p;
            ++it;
        } while (p != temp);
    }
    template<typename T >
    bool operator==(const T & other) const {
        return other.m_value == m_value;
    }

    template <typename Iter>
    void Deserialise(Iter &it, Iter end) {
        skipWS(it, end);
        if(it == end) return;
        char buf[20];
        int l = 0;
        char s = *it;
        while(!std::isspace(s) && s != '}' && s != ']' && s != ','&&it != end) {
            buf[l] = s;
            ++it;
            ++l;
            s = *it;
        }
        if(it == end) return;
        buf[l] = 0;

        char *ptr = 0;
        m_value = std::strtoll(buf, &ptr, 10);
        if(ptr == buf) {
            m_value = 0;
            it= end;
            return;
        }
        skipWS(it, end);
        wasSet = true;
    }
};

using Int = BaseInt<10, int32_t>;



template <typename ...Members>
class Array {
protected:
    std::tuple<Members...> m_members;

    template<typename OtherT, std::size_t ... Is>
    bool cmp_helper(const OtherT &other, std::index_sequence<Is...>) const {
        return (true && ... && (std::get<Is>(other.m_members) == std::get<Is>(m_members)));
    }
public:
    bool wasSet = false;
    void ClearSetFlag() {
        wasSet = false;
        foreach([](auto &ch){ch.ClearSetFlag();});
    }
    static constexpr std::size_t  Length = sizeof... (Members);

    static constexpr std::size_t MaxStrSize = 2 + Length - 1 + (0 + ... + Members::MaxStrSize);
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
    void Serialise(Iter & it) {
        *it = '[';
        ++it;
        if constexpr (Length > 0) {
            bool first = true;
            at<0>().Serialise(it);
            foreach([&](auto &m) {
                    if(first) {
                    first = false;
                    return;
        }
                    *it = ',';
                    ++it;
                    m.Serialise(it);
        });
        }
        *it  = ']';
        ++it;
    }
    template <typename OtherT>
    bool operator==(const OtherT & other) const {
        return cmp_helper(other, std::make_index_sequence<Length>());
    }

    template <typename Iter>
    void Deserialise(Iter &it, Iter end) {
        ClearSetFlag();
        skipWSUntil(it, end, '[');
        if(it == end) return;
        ++it;
        if(it == end) {return;}
        skipWS(it, end);
        if(it == end) {return;}
        if(*it == ']') {
            wasSet = true;
            ++it; return;
        }
        std::size_t filled = 0;
        bool err = false;

        foreach(
                [&](auto &v){
                if(err || *it == ']') return ;
                v.Deserialise(it, end);
                if(it == end) {err = true; return;}
                skipWS(it, end);
                if(it == end) {err = true; return;}
                if(*it == ',') {
                    ++it;
                    if(it == end) {err = true; return;}
                }else if(*it != ']')  {
                    it = end; return;
                }
                skipWS(it, end);
                if(it == end) {err = true; return;}
        });

        if (filled < Length) {
            //TODO mark other fields as uninit
        }
        if(err) {it = end; return;}
        while(*it != ']' && it != end) {
            skipWS(it, end);
            if(it == end) return;
            skipJSON(it, end, 0);
            if(it == end) return;
            skipWS(it, end);
            if(*it == ',') {
                ++it;
                if(it == end) {err = true; return;}
            }else if(*it != ']')  {
                it = end; return;
            }
        }
        if(*it != ']' ) {it = end; return;}
        if(it == end) return;
        ++it;
        skipWS(it, end);
        wasSet = true;
        return;
    }
};
template <typename ... Members>
class Array<std::tuple<Members...>>:public Array<Members...> {};

template <typename SSName, typename Obj>
struct N{
    using Name = SSName;
    using MemberT = Obj;
    MemberT member;

    static constexpr std::size_t MaxStrSize =
            + MemberT::MaxStrSize // value
            + Name::Size //name
            + ( 1 + 2) // colon and quotes
            ;
    template <typename Iter>
    void Serialise(Iter & it) {
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

    template <typename OtherT>
    bool operator==(const OtherT & other)const  {
        return compare_v<Name, OtherT::Name> && member == other;
    }
};

template <typename OutPairs, typename FieldPairMembers, typename TT = void>
struct repacker{};

template <typename ...OutPairs, typename Name, typename Member, typename ...rest>
struct repacker<std::tuple<OutPairs...>,  std::tuple<Name, Member, rest...>, enable_if_t<is_ss_v<Name>>> :
        repacker<std::tuple<OutPairs..., N<Name, Member>>, std::tuple<rest...>>
{
    static_assert ( is_ss_v<Name>);
};

template <typename ...OutPairs, typename LastName, typename LastMember>
struct repacker<std::tuple<OutPairs...>, std::tuple<LastName, LastMember>, enable_if_t<is_ss_v<LastName>>>{
    using type = std::tuple<OutPairs..., N<LastName, LastMember>>;
    using names =  std::tuple<typename OutPairs::Name..., LastName>;
};

template <typename ... PairsNames, typename ... PairsMembers>
struct repacker<std::tuple<>, std::tuple<N<PairsNames, PairsMembers>...>, enable_if_t<sizeof... (PairsNames) == sizeof... (PairsNames)>>{
                                                                                                                                        using type = std::tuple<N<PairsNames, PairsMembers>...>;
                                                                                                                                        using names =  std::tuple<PairsNames...>;
};


template <typename ...Members>
class Object {
public:
    using repacked = repacker<std::tuple<>, std::tuple<Members...>>;
    using MemberTupleT =  typename repacked::type;
    using Names = typename repacked::names;
    MemberTupleT m_members;


    template <typename> struct IsWithName : std::false_type { };
    template <typename SS, typename MemberT> struct IsWithName<N<SS, MemberT>> : std::true_type { };

    //    static_assert((true && ... && IsWithName<Members>::value), "Please, use N wrapper to create object members");

    using Trie = StaticTrie::Trie<Names>;
private:
    template<typename OtherT, std::size_t ... Is>
    bool cmp_helper(const OtherT &other, std::index_sequence<Is...>) const {
        //        using OT = std::decay_t<OtherT>;
        bool namesCorrect =  (true && ... && ( compare_v<typename tuple_element_t<Is, typename OtherT::MemberTupleT>::Name, typename tuple_element_t<Is, MemberTupleT>::Name>));
        bool valsCorrect =  (true && ... && (std::get<Is>(other.m_members).member == std::get<Is>(m_members).member));
        return namesCorrect && valsCorrect;
    }
    template <typename M1, typename M2> struct comparator {constexpr static bool value = M1::Name::Size < M2::Name::Size;};


    constexpr static size_t MinKeyL = min_finder_t<MemberTupleT, comparator>::Name::Size;
    constexpr static size_t MaxKeyL = max_finder_t<MemberTupleT, comparator>::Name::Size;

    template <typename S> struct sizeCounter{};
    template <typename ...S> struct sizeCounter<std::tuple<S...>>{static constexpr std::size_t MaxStrSize = (0 + ... + S::MaxStrSize);};

public:

    bool wasSet = false;
    void ClearSetFlag() {
        wasSet = false;
        foreach([](auto &ch){ch.member.ClearSetFlag();});
    }
    static constexpr std::size_t  Length = std::tuple_size_v<MemberTupleT>;
    static constexpr std::size_t MaxStrSize = 2 + Length - 1 + sizeCounter<MemberTupleT>::MaxStrSize;

    template<typename SS>
    auto & at() {
        static_assert (is_ss_v<SS>, "Use static string as key");
        static_assert (type_in_tuple_v<SS, Names>, "Static json object property not found");
        return std::get<tuple_first_type_index_v<SS, Names>>(m_members).member;
    }
    template<typename Callable>
    void foreach(Callable &&c) {
        iterateTuple(m_members, c);
    }

    template <typename Iter>
    void Serialise(Iter & it) {
        *it = '{';
        ++it;
        if constexpr (Length > 0) {
            bool first = true;
            std::get<0>(m_members).Serialise(it);
            foreach([&](auto &m) {
                    if(first) {
                    first = false;
                    return;
            }
                    *it = ',';
                    ++it;
                    m.Serialise(it);
        });
        }
        *it  = '}';
        ++it;
    }


    template <typename Iter>
    void Deserialise(Iter& it, Iter end) {
        ClearSetFlag();
        skipWSUntil(it, end, '{');
        if(it == end) return;
        ++it;
        skipWS(it, end);
        if(*it == '}') {
            wasSet = true;
            ++it; return;
        }
        //props finding
        bool fieldConsumed = false;
        std::size_t index = 0;
        auto clb = [&](auto matchInfo) -> char {
            index ++;
//            using MatchInfo = decltype (matchInfo);
            //            cout << "Matched: " << endl;
            //            iterateTuple(typename MatchInfo::MatchedStrings(), [](auto s){
            //                cout << "\t " << decltype (s)::ItemT::to_str() << endl;
            //            });
            //            cout << endl;
            //            if constexpr(matchInfo.hasFull) {
            //                cout << "Full Node string: " << MatchInfo::NodeString::to_str() << endl;
            //            }

            if constexpr(matchInfo.hasFull) {
                if(it == end) return -1;
                if constexpr(matchInfo.isLast) {
                    while(index-1 < std::tuple_element_t<matchInfo.index, Names>::Size && it != end) {
                        if (std::tuple_element_t<matchInfo.index, Names>::to_str()[index-1] != *it) {
                            return -1;
                        }
                        ++index;
                        ++ it;
                    }
                }
                if(it == end) return -1;
                if(*it != '"') {
                    return *it;
                }

                ++it;
                if(it == end) return -1;
                skipWSUntil(it, end, ':');
                if(it == end) return -1;
                ++it;
                skipWS(it, end);
                if(it == end) return -1;
                std::get<matchInfo.index>(m_members).member.Deserialise(it, end);
                fieldConsumed = true;
                return -1;
            } else
            ++it;

            return *it;
        };

        do {
            index = 0;
            fieldConsumed = false;
            skipWSUntil(it, end, '"');
            if(it == end) return;
            ++it;
            Trie::search(*it, clb);
            if(!fieldConsumed) {
                while(*it != '"' && it != end) ++it;
                if(it == end) return;
                ++it;
                skipWS(it, end);
                if (*it != ':') {it = end; return;}
                ++it;
                skipWS(it, end);
                if(it == end) return;

                skipJSON(it, end, 0);
                if(it == end) return;
            }
            if(it == end) return;
            skipWS(it, end);
            if(it == end) return;
            if(*it == ',') {
                ++it;
                skipWS(it, end);
            } else if(*it != '}')  {
                it = end; return;
            }
        } while(*it != '}' && it!=end);
        if(*it != '}')  {it = end; return;}
        ++it;
        skipWS(it, end);
        wasSet = true;
        return;
    }
    template <typename OtherT>
    bool operator==(const OtherT & other) const {
        return cmp_helper(other, std::make_index_sequence<Length>());
    }

    //    void print(int indent) {
    //        std;
    //    }
};

template <typename RootObj>
class Serialiser {
    RootObj &m_obj;

public:
    Serialiser(RootObj & obj):m_obj(obj) {}
    static constexpr std::size_t MaxStrSize = RootObj::MaxStrSize;

    template<typename Iter>
    void Serialise(Iter it) {
        m_obj.Serialise(it);
    }
};
}

#endif // STATIC_JSON_HPP
