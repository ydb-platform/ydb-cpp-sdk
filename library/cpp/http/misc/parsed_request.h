#pragma once

#include <util/generic/strbuf.h>

struct TParsedHttpRequest {
    TParsedHttpRequest(const std::string_view& str);

    std::string_view Method;
    std::string_view Request;
    std::string_view Proto;
};

struct TParsedHttpLocation {
    TParsedHttpLocation(const std::string_view& req);

    std::string_view Path;
    std::string_view Cgi;
};

struct TParsedHttpFull: public TParsedHttpRequest, public TParsedHttpLocation {
    inline TParsedHttpFull(const std::string_view& line)
        : TParsedHttpRequest(line)
        , TParsedHttpLocation(Request)
    {
    }
};
