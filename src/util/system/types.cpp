#include <ydb-cpp-sdk/util/system/types.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/typetraits.h>
#include <ydb-cpp-sdk/util/generic/typelist.h>
=======
#include <src/util/generic/typetraits.h>
#include <src/util/generic/typelist.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/typetraits.h>
#include <ydb-cpp-sdk/util/generic/typelist.h>
>>>>>>> origin/main

static_assert(sizeof(ui8) == 1, "incorrect ui8 type");
static_assert(sizeof(ui16) == 2, "incorrect ui16 type");
static_assert(sizeof(ui32) == 4, "incorrect ui32 type");
static_assert(sizeof(ui64) == 8, "incorrect ui64 type");

static_assert(sizeof(i8) == 1, "incorrect i8 type");
static_assert(sizeof(i16) == 2, "incorrect i16 type");
static_assert(sizeof(i32) == 4, "incorrect i32 type");
static_assert(sizeof(i64) == 8, "incorrect i64 type");

static_assert(sizeof(size_t) == sizeof(ssize_t), "incorrect ssize_t");

static_assert(TTypeList<ui32, ui64>::THave<size_t>::value, "incorrect size_t");
