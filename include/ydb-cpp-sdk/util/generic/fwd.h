#pragma once

<<<<<<< HEAD
=======
<<<<<<<< HEAD:src/util/generic/fwd.h
>>>>>>> origin/main
<<<<<<<< HEAD:include/ydb-cpp-sdk/util/generic/fwd.h
#include <ydb-cpp-sdk/util/system/defaults.h>
========
#include <src/util/system/defaults.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/generic/fwd.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/util/generic/fwd.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD
=======
========
#include <ydb-cpp-sdk/util/system/defaults.h>
>>>>>>>> origin/main:include/ydb-cpp-sdk/util/generic/fwd.h
>>>>>>> origin/main

#include <iosfwd>
#include <vector>
#include <deque>

//misc
class TBuffer;

//functors
template <class T = void>
struct TLess;

template <class T = void>
struct TGreater;

template <class T = void>
struct TEqualTo;

template <class T>
struct THash;

//intrusive containers
struct TIntrusiveListDefaultTag;
template <class T, class Tag = TIntrusiveListDefaultTag>
class TIntrusiveList;

template <class T, class D, class Tag = TIntrusiveListDefaultTag>
class TIntrusiveListWithAutoDelete;

template <class T, class Tag = TIntrusiveListDefaultTag>
class TIntrusiveSList;

template <class TValue, class TCmp>
class TRbTree;

//containers
//template <class T, class A = std::allocator<T>>
//class TDeque;

template <class Key, class T, class HashFcn = THash<Key>, class EqualKey = TEqualTo<Key>, class Alloc = std::allocator<Key>>
class THashMap;

template <class Key, class T, class HashFcn = THash<Key>, class EqualKey = TEqualTo<Key>, class Alloc = std::allocator<Key>>
class THashMultiMap;

template <class K, class L = TLess<K>, class A = std::allocator<K>>
class TSet;

template <class K, class L = TLess<K>, class A = std::allocator<K>>
class TMultiSet;

template <class T, class S = std::deque<T>>
class TStack;

template <size_t BitCount, typename TChunkType = ui64>
class TBitMap;

//autopointers
class TDelete;
class TDeleteArray;
class TFree;
class TCopyNew;

template <class T, class D = TDelete>
class TAutoPtr;

template <class T, class D = TDelete>
class THolder;

template <class T, class C, class D = TDelete>
class TRefCounted;

template <class T>
class TDefaultIntrusivePtrOps;

template <class T, class Ops>
class TSimpleIntrusiveOps;

template <class T, class Ops = TDefaultIntrusivePtrOps<T>>
class TIntrusivePtr;

template <class T, class Ops = TDefaultIntrusivePtrOps<T>>
class TIntrusiveConstPtr;

template <class T, class Ops = TDefaultIntrusivePtrOps<T>>
using TSimpleIntrusivePtr = TIntrusivePtr<T, TSimpleIntrusiveOps<T, Ops>>;

template <class T, class C, class D = TDelete>
class TSharedPtr;

template <class T, class C = TCopyNew, class D = TDelete>
class TCopyPtr;

template <class TPtr, class TCopy = TCopyNew>
class TCowPtr;

template <typename T>
class TPtrArg;

template <typename T>
using TArrayHolder = THolder<T, TDeleteArray>;

template <typename T>
using TMallocHolder = THolder<T, TFree>;

template <typename T>
using TArrayPtr = TAutoPtr<T, TDeleteArray>;

template <typename T>
using TMallocPtr = TAutoPtr<T, TFree>;

struct TGUID;
