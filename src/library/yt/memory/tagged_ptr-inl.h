#ifndef TAGGED_PTR_INL_H_
#error "Direct inclusion of this file is not allowed, include tagged_ptr.h"
// For the sake of sane code completion.
#include "tagged_ptr.h"
#endif

#include <src/library/yt/assert/assert.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/compiler.h>
=======
#include <src/util/system/compiler.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

template <class T>
Y_FORCE_INLINE TTaggedPtr<T>::TTaggedPtr(T* ptr, ui16 tag)
    : Ptr(ptr)
    , Tag(tag)
{ }

template <class T>
Y_FORCE_INLINE TTaggedPtr<T>::TTaggedPtr(T* ptr)
    : Ptr(ptr)
    , Tag(0)
{ }

template <class T>
Y_FORCE_INLINE TPackedPtr TTaggedPtr<T>::Pack() const
{
    YT_ASSERT((reinterpret_cast<TPackedPtr>(Ptr) & PackedPtrTagMask) == 0);
    return (static_cast<TPackedPtr>(Tag) << PackedPtrAddrsssBits) | reinterpret_cast<TPackedPtr>(Ptr);
}

template <class T>
Y_FORCE_INLINE TTaggedPtr<T> TTaggedPtr<T>::Unpack(TPackedPtr packedPtr)
{
    return {
        reinterpret_cast<T*>(packedPtr & PackedPtrAddressMask),
        static_cast<ui16>(packedPtr >> PackedPtrAddrsssBits),
    };
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
