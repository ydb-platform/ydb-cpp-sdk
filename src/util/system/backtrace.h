#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/util/system/backtrace.h
#include <ydb-cpp-sdk/util/system/defaults.h>

========
#include <src/util/generic/fwd.h>
#include <src/util/system/defaults.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/system/backtrace.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/util/system/backtrace.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
#include <span>
#include <string>

class IOutputStream;

size_t BackTrace(void** p, size_t len);

struct TResolvedSymbol {
    const char* Name;
    void* NearestSymbol;
};

TResolvedSymbol ResolveSymbol(void* sym, char* buf, size_t len);

void FormatBackTrace(IOutputStream* out, void* const* backtrace, size_t backtraceSize);
void FormatBackTrace(IOutputStream* out);
void PrintBackTrace();

using TFormatBackTraceFn = void (*)(IOutputStream*, void* const* backtrace, size_t backtraceSize);

TFormatBackTraceFn SetFormatBackTraceFn(TFormatBackTraceFn f);
TFormatBackTraceFn GetFormatBackTraceFn();

using TBackTraceView = std::span<void* const>;

class TBackTrace {
private:
    static constexpr size_t CAPACITY = 300;
    void* Data[CAPACITY];
    size_t Size;

public:
    TBackTrace();
    void Capture();
    void PrintTo(IOutputStream&) const;
    std::string PrintToString() const;
    size_t size() const;
    const void* const* data() const;
    operator TBackTraceView() const;

    static TBackTrace FromCurrentException();
};
