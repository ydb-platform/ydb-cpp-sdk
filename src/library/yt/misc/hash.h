#pragma once

#include <src/util/generic/hash.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/random/random.h>
=======
#include <src/util/random/random.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/random/random.h>
>>>>>>> origin/main

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

//! Updates #h with #k.
//! Cf. |boost::hash_combine|.
void HashCombine(size_t& h, size_t k);

//! Updates #h with the hash of #k.
//! Cf. |boost::hash_combine|.
template <class T>
void HashCombine(size_t& h, const T& k);

////////////////////////////////////////////////////////////////////////////////

//! Provides a hasher that randomizes the results of another one.
template <class TElement, class TUnderlying = ::THash<TElement>>
class TRandomizedHash
{
public:
    TRandomizedHash();
    size_t operator () (const TElement& element) const;

private:
    size_t Seed_;
    TUnderlying Underlying_;

};

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT

#define HASH_INL_H_
#include "hash-inl.h"
#undef HASH_INL_H_
