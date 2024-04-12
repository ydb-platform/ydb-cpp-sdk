#include "engine.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/string/cast.h>
#include <ydb-cpp-sdk/util/string/escape.h>
=======
#include <src/util/string/cast.h>
#include <src/util/string/escape.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#if !defined(DBGDUMP_INLINE_IF_INCLUDED)
#define DBGDUMP_INLINE_IF_INCLUDED
#endif

DBGDUMP_INLINE_IF_INCLUDED void TDumpBase::String(const std::string_view& s) {
    if (!s.empty()) {
        Raw(NUtils::Quote(s));
    } else {
        Raw("(empty)");
    }
}

DBGDUMP_INLINE_IF_INCLUDED void TDumpBase::String(const std::u16string_view& s) {
    Raw("w");
    String(ToString(s));
}

DBGDUMP_INLINE_IF_INCLUDED void TDumpBase::Raw(const std::string_view& s) {
    Stream().Write(s.data(), s.size());
}

DBGDUMP_INLINE_IF_INCLUDED void TDumpBase::Char(char ch) {
    Raw("'" + EscapeC(&ch, 1) + "'");
}

DBGDUMP_INLINE_IF_INCLUDED void TDumpBase::Char(wchar16 ch) {
    Raw("w'" + ToString(EscapeC(&ch, 1)) + "'");
}
