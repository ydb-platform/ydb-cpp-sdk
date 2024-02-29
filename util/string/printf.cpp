#include "printf.h"

#include <util/stream/printf.h>
#include <util/stream/str.h>

int vsprintf(std::string& s, const char* c, va_list params) {
    TStringOutput so(s.remove());

    return Printf(so, c, params);
}

int sprintf(std::string& s, const char* c, ...) {
    va_list params;
    va_start(params, c);
    const int k = vsprintf(s, c, params);
    va_end(params);
    return k;
}

std::string Sprintf(const char* c, ...) {
    std::string s;
    va_list params;
    va_start(params, c);
    vsprintf(s, c, params);
    va_end(params);
    return s;
}

int fcat(std::string& s, const char* c, ...) {
    TStringOutput so(s);

    va_list params;
    va_start(params, c);
    const size_t ret = Printf(so, c, params);
    va_end(params);

    return ret;
}
