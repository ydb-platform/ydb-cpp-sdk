#include "parsed_request.h"

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> origin/main
#include <ydb-cpp-sdk/library/string_utils/misc/misc.h>

#include <ydb-cpp-sdk/util/string/escape.h>
#include <ydb-cpp-sdk/util/string/strip.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>
#include <ydb-cpp-sdk/util/string/cast.h>
<<<<<<< HEAD
=======
#include <src/library/string_utils/misc/misc.h>

#include <src/util/string/escape.h>
#include <src/util/string/strip.h>
#include <src/util/generic/yexception.h>
#include <src/util/string/cast.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main

static inline std::string_view StripLeft(const std::string_view& s) noexcept {
    const char* b = s.begin();
    const char* e = s.end();

    StripRangeBegin(b, e);

    return std::string_view(b, e);
}

TParsedHttpRequest::TParsedHttpRequest(const std::string_view& str) {
    std::string_view tmp;

    if (!NUtils::TrySplit(StripLeft(str), Method, tmp, ' ')) {
        ythrow yexception() << "bad request(" << NUtils::Quote(str) << ")";
    }

    if (!NUtils::TrySplit(StripLeft(tmp), Request, Proto, ' ')) {
        ythrow yexception() << "bad request(" << NUtils::Quote(str) << ")";
    }

    Proto = StripLeft(Proto);
}

TParsedHttpLocation::TParsedHttpLocation(const std::string_view& req) {
    NUtils::Split(req, Path, Cgi, '?');
}
