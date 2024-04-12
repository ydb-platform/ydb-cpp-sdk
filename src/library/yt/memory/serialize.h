#pragma once

#include "intrusive_ptr.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/ysaveload.h>
=======
#include <src/util/ysaveload.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

////////////////////////////////////////////////////////////////////////////////

template <class T>
class TSerializer<NYT::TIntrusivePtr<T>>
{
public:
    static inline void Save(IOutputStream* output, const NYT::TIntrusivePtr<T>& ptr);
    static inline void Load(IInputStream* input, NYT::TIntrusivePtr<T>& ptr);
};

////////////////////////////////////////////////////////////////////////////////

#define SERIALIZE_PTR_INL_H_
#include "serialize-inl.h"
#undef SERIALIZE_PTR_INL_H_
