#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/datetime/base.h>
=======
#include <src/util/datetime/base.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

#include <ctime>

#define BAD_DATE ((time_t)-1)

inline time_t parse_http_date(const std::string_view& datestring) {
    try {
        return TInstant::ParseHttpDeprecated(datestring).TimeT();
    } catch (const TDateTimeParseException&) {
        return BAD_DATE;
    }
}

int format_http_date(char buf[], size_t size, time_t when);
char* format_http_date(time_t when, char* buf, size_t len);

std::string FormatHttpDate(time_t when);
