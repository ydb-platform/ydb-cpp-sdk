#include "parsed_request.h"

#include <ydb-cpp-sdk/library/string_utils/misc/misc.h>

#include <src/util/string/escape.h>
#include <src/util/string/strip.h>
#include <src/util/generic/yexception.h>
#include <src/util/string/cast.h>

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
