#include "other.h"

#include <src/util/string/util.h>

#include <ydb-cpp-sdk/library/string_utils/helpers/helpers.h>

#include <ydb-cpp-sdk/util/generic/utility.h>
#include <ydb-cpp-sdk/util/system/compat.h>
#include <ydb-cpp-sdk/util/system/yassert.h>

/********************************************************/
/********************************************************/

static const Tr InvertTr(".:/?#", "\005\004\003\002\001");
static const Tr RevertTr("\005\004\003\002\001", ".:/?#");

void TrspChars(char* s) {
    InvertTr.Do(s);
}

void UnTrspChars(char* s) {
    RevertTr.Do(s);
}

void TrspChars(char* s, size_t l) {
    InvertTr.Do(s, l);
}

void UnTrspChars(char* s, size_t l) {
    RevertTr.Do(s, l);
}

void TrspChars(const char* s, char* d) {
    InvertTr.Do(s, d);
}

void UnTrspChars(const char* s, char* d) {
    RevertTr.Do(s, d);
}

void InvertDomain(char* begin, char* end) {
    // skip schema if it is present
    const auto dotPos = std::string_view{begin, end}.find('.');
    if (dotPos == std::string_view::npos) {
        return; // no need to invert anything
    }
    const auto schemaendPos = std::string_view{begin, end}.find("://", 3);
    if (schemaendPos < dotPos) {
        begin += schemaendPos + 3;
    }
    char* sl = static_cast<char*>(memchr(begin, '/', end - begin));
    char* cl = static_cast<char*>(memchr(begin, ':', sl ? sl - begin : end - begin));
    end = cl ? cl : (sl ? sl : end);

    // invert string
    for (size_t i = 0, n = end - begin; i < n / 2; ++i) {
        DoSwap(begin[i], begin[n - i - 1]);
    }

    // invert back each host name segment
    char* b = begin;
    while (true) {
        char* e = static_cast<char*>(memchr(b, '.', end - b));
        if (!e) {
            e = end;
        }
        for (size_t i = 0, n = e - b; i < n / 2; ++i) {
            DoSwap(b[i], b[n - i - 1]);
        }
        if (e == end) {
            break;
        }
        b = e + 1;
    }
}

void InvertUrl(char* begin, char* end) {
    char* slash = strchr(begin, '/');
    if (slash) {
        *slash = 0;
    }
    NUtils::ToLower(begin);
    if (slash) {
        *slash = '/';
    }
    InvertDomain(begin, end);
    TrspChars(begin);
}

void RevertUrl(char* begin, char* end) {
    UnTrspChars(begin);
    InvertDomain(begin, end);
}
