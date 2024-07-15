#pragma once

#include "engine.h"

#include <ydb-cpp-sdk/util/generic/fwd.h>

#include <list>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include <memory>

//smart pointers
template <class T, class D>
struct TDumper<std::unique_ptr<T, D>> {
    template <class S>
    static inline void Dump(S& s, const std::unique_ptr<T, D>& v) {
        s << DumpRaw("std::unique_ptr(") << v.get() << DumpRaw(")");
    }
};

template <class T, class D>
struct TDumper<THolder<T, D>> {
    template <class S>
    static inline void Dump(S& s, const THolder<T, D>& v) {
        s << DumpRaw("THolder(") << v.Get() << DumpRaw(")");
    }
};

template <class T, class Ops>
struct TDumper<TIntrusivePtr<T, Ops>> {
    template <class S>
    static inline void Dump(S& s, const TIntrusivePtr<T, Ops>& v) {
        s << DumpRaw("TIntrusivePtr(") << v.Get() << DumpRaw(")");
    }
};

template <class T>
struct TDumper<std::shared_ptr<T>> {
    template <class S>
    static inline void Dump(S& s, const std::shared_ptr<T>& v) {
        s << DumpRaw("std::shared_ptr(") << v.get() << DumpRaw(")");
    }
};

template <class T, class C, class D>
struct TDumper<TCopyPtr<T, C, D>> {
    template <class S>
    static inline void Dump(S& s, const TCopyPtr<T, C, D>& v) {
        s << DumpRaw("TCopyPtr(") << v.Get() << DumpRaw(")");
    }
};

//small ints
// Default dumper prints them via IOutputStream << (value), which results in raw
// chars, not integer values. Cast to a bigger int type to force printing as
// integers.
// NB: i8 = signed char != char != unsigned char = ui8
template <>
struct TDumper<ui8>: public TDumper<i32> {
};

template <>
struct TDumper<i8>: public TDumper<i32> {
};

//chars
template <>
struct TDumper<char>: public TCharDumper {
};

template <>
struct TDumper<wchar16>: public TCharDumper {
};

//pairs
template <class A, class B>
struct TDumper<std::pair<A, B>> {
    template <class S>
    static inline void Dump(S& s, const std::pair<A, B>& v) {
        s.ColorScheme.Key(s);
        s.ColorScheme.Literal(s);
        s << v.first;
        s.ColorScheme.ResetType(s);
        s.ColorScheme.ResetRole(s);
        s.ColorScheme.Markup(s);
        s << DumpRaw(" -> ");
        s.ColorScheme.Value(s);
        s.ColorScheme.Literal(s);
        s << v.second;
        s.ColorScheme.ResetType(s);
        s.ColorScheme.ResetRole(s);
    }
};

//sequences
template <class T, class A>
struct TDumper<std::vector<T, A>>: public TSeqDumper {
};

template <class T>
struct TDumper<std::span<T>>: public TSeqDumper {
};

template <class T, size_t N>
struct TDumper<std::array<T, N>>: public TSeqDumper {
};

template <class T, class A>
struct TDumper<std::deque<T, A>>: public TSeqDumper {
};

template <class T, class A>
struct TDumper<std::list<T, A>>: public TSeqDumper {
};

//associatives
template <class K, class V, class P, class A>
struct TDumper<std::map<K, V, P, A>>: public TAssocDumper {
};

template <class K, class V, class P, class A>
struct TDumper<std::multimap<K, V, P, A>>: public TAssocDumper {
};

template <class T, class P, class A>
struct TDumper<std::set<T, P, A>>: public TAssocDumper {
};

template <class T, class P, class A>
struct TDumper<std::multiset<T, P, A>>: public TAssocDumper {
};

template <class K, class V, class H, class P, class A>
struct TDumper<std::unordered_map<K, V, H, P, A>>: public TAssocDumper {
};

template <class K, class V, class H, class P, class A>
struct TDumper<std::unordered_multimap<K, V, H, P, A>>: public TAssocDumper {
};

template <class T, class H, class P, class A>
struct TDumper<std::unordered_set<T, H, P, A>>: public TAssocDumper {
};

template <class T, class H, class P, class A>
struct TDumper<std::unordered_multiset<T, H, P, A>>: public TAssocDumper {
};

//strings
template <>
struct TDumper<std::string>: public TStrDumper {
};

template <>
struct TDumper<const char*>: public TStrDumper {
};

template <>
struct TDumper<const wchar16*>: public TStrDumper {
};

template <class C, class T, class A>
struct TDumper<std::basic_string<C, T, A>>: public TStrDumper {
};

template <class TChar>
struct TDumper<std::basic_string_view<TChar>>: public TStrDumper {
};
