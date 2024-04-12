#include "defaults.h"
#include "progname.h"
#include "compat.h"
#include "error.h"

#include <src/util/generic/scope.h>

#include <src/util/system/compat.h>

#include <iostream>

namespace {
    void PrintWithFormat(std::ostream& out, const char* fmt, va_list args) {
        va_list args_copy;
        va_copy(args_copy, args);
        int len = std::vsprintf(nullptr, fmt, args_copy);
        va_end(args_copy);
        std::string tmp(len + 1, '\0');
        std::vsnprintf(tmp.data(), len + 1, fmt, args);
        tmp.resize(len);
        out << tmp;
    }
}

void vwarnx(const char* fmt, va_list args) {
    std::cerr << GetProgramName() << ": ";

    if (fmt) {
        PrintWithFormat(std::cerr, fmt, args);
    }

    std::cerr << '\n';
}

void vwarn(const char* fmt, va_list args) {
    int curErrNo = errno;
    auto curErrText = LastSystemErrorText();

    Y_DEFER {
        errno = curErrNo;
    };

    std::cerr << GetProgramName() << ": ";

    if (fmt) {
        PrintWithFormat(std::cerr, fmt, args);
        std::cerr << ": ";
    }

    std::cerr << curErrText << '\n';
}

void warn(const char* fmt, ...) {
    va_list args;

    va_start(args, fmt);
    vwarn(fmt, args);
    va_end(args);
}

void warnx(const char* fmt, ...) {
    va_list args;

    va_start(args, fmt);
    vwarnx(fmt, args);
    va_end(args);
}

[[noreturn]] void verr(int status, const char* fmt, va_list args) {
    vwarn(fmt, args);
    std::exit(status);
}

[[noreturn]] void err(int status, const char* fmt, ...) {
    va_list args;

    va_start(args, fmt);
    verr(status, fmt, args);
    va_end(args);
}

[[noreturn]] void verrx(int status, const char* fmt, va_list args) {
    vwarnx(fmt, args);
    std::exit(status);
}

[[noreturn]] void errx(int status, const char* fmt, ...) {
    va_list args;

    va_start(args, fmt);
    verrx(status, fmt, args);
    va_end(args);
}
