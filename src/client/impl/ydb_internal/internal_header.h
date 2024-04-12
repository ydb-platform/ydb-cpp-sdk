#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/compiler.h>
=======
#include <src/util/system/compiler.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#if !defined(INCLUDE_YDB_INTERNAL_H)
#error "you are trying to use internal ydb header"
#endif // INCLUDE_YDB_INTERNAL_H

// This macro allow to send credentials data via insecure channel
#define YDB_GRPC_UNSECURE_AUTH

// This macro is used for grpc debug purpose only
//#define YDB_GRPC_BYPASS_CHANNEL_POOL

// gRpc issue temporal workaround.
// In case of early TryCancel call and sharing shannel interfaces
// grpc doesn`t free allocated memory at the client shutdown
#if defined(_asan_enabled_)
#define YDB_GRPC_BYPASS_CHANNEL_POOL
#endif
