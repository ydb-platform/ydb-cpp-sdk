#include "assert.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/yassert.h>
#include <ydb-cpp-sdk/util/system/compiler.h>
=======
#include <src/util/system/yassert.h>
#include <src/util/system/compiler.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

namespace NYT::NDetail {

////////////////////////////////////////////////////////////////////////////////

Y_WEAK void AssertTrapImpl(
    std::string_view trapType,
    std::string_view expr,
    std::string_view file,
    int line,
    std::string_view function)
{
    // Map to Arcadia assert, poorly...
    ::NPrivate::Panic(
        ::NPrivate::TStaticBuf(file.data(), file.length()),
        line,
        function.data(),
        expr.data(),
        "%s",
        trapType.data());
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NDetail
