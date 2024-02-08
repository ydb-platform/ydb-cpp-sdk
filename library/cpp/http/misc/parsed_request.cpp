#include "parsed_request.h"

#include <util/string/strip.h>
#include <util/generic/yexception.h>
#include <util/string/cast.h>

static inline std::string_view StripLeft(const std::string_view& s) noexcept {
    const char* b = s.begin();
    const char* e = s.end();

    StripRangeBegin(b, e);

    return std::string_view(b, e);
}

TParsedHttpRequest::TParsedHttpRequest(const std::string_view& str) {
    std::string_view tmp;

    if (!StripLeft(str).TrySplit(' ', Method, tmp)) {
        ythrow yexception() << "bad request(" << ToString(str).Quote() << ")";
    }

    if (!StripLeft(tmp).TrySplit(' ', Request, Proto)) {
        ythrow yexception() << "bad request(" << ToString(str).Quote() << ")";
    }

    Proto = StripLeft(Proto);
}

TParsedHttpLocation::TParsedHttpLocation(const std::string_view& req) {
    req.Split('?', Path, Cgi);
}
