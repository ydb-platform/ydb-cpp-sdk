#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/types.h>
=======
#include <src/util/system/types.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

using TPackedPtr = uintptr_t;
static_assert(sizeof(TPackedPtr) == 8);

constexpr size_t PackedPtrAddrsssBits = 48;
constexpr size_t PackedPtrTagBits = 16;
constexpr TPackedPtr PackedPtrAddressMask = (1ULL << PackedPtrAddrsssBits) - 1;
constexpr TPackedPtr PackedPtrTagMask = ~PackedPtrAddressMask;

////////////////////////////////////////////////////////////////////////////////

template <class T>
struct TTaggedPtr
{
    TTaggedPtr() = default;
    TTaggedPtr(T* ptr, ui16 tag);
    explicit TTaggedPtr(T* ptr);

    T* Ptr;
    ui16 Tag;

    TPackedPtr Pack() const;
    static TTaggedPtr<T> Unpack(TPackedPtr packedPtr);
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT

#define TAGGED_PTR_INL_H_
#include "tagged_ptr-inl.h"
#undef TAGGED_PTR_INL_H_
