#include <ydb-cpp-sdk/util/stream/output.h>

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> origin/main
#include <ydb-cpp-sdk/util/string/cast.h>
#include "format.h"
#include <ydb-cpp-sdk/util/memory/tempbuf.h>
#include <ydb-cpp-sdk/util/generic/singleton.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>
<<<<<<< HEAD
=======
#include <src/util/string/cast.h>
#include "format.h"
#include <src/util/memory/tempbuf.h>
#include <src/util/generic/singleton.h>
#include <src/util/generic/yexception.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
#include <src/util/charset/utf8.h>
#include <src/util/charset/wide.h>

#if defined(_android_)
    #include <src/util/system/dynlib.h>
<<<<<<< HEAD
<<<<<<< HEAD
    #include <ydb-cpp-sdk/util/system/guard.h>
=======
    #include <src/util/system/guard.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
    #include <ydb-cpp-sdk/util/system/guard.h>
>>>>>>> origin/main
    #include <android/log.h>
    #include <mutex>
#endif

#include <cerrno>
#include <cstdio>
#include <filesystem>
#include <string_view>
#include <optional>

#if defined(_win_)
    #include <io.h>
#endif

constexpr size_t MAX_UTF8_BYTES = 4; // UTF-8-encoded code point takes between 1 and 4 bytes

IOutputStream::IOutputStream() noexcept = default;

IOutputStream::~IOutputStream() = default;

void IOutputStream::DoFlush() {
    /*
     * do nothing
     */
}

void IOutputStream::DoFinish() {
    Flush();
}

void IOutputStream::DoWriteV(const TPart* parts, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        const TPart& part = parts[i];

        DoWrite(part.buf, part.len);
    }
}

void IOutputStream::DoWriteC(char ch) {
    DoWrite(&ch, 1);
}

template <>
void Out<wchar16>(IOutputStream& o, wchar16 ch) {
    const wchar32 w32ch = ReadSymbol(&ch, &ch + 1);
    size_t length;
    unsigned char buffer[MAX_UTF8_BYTES];
    WriteUTF8Char(w32ch, length, buffer);
    o.Write(buffer, length);
}

template <>
void Out<wchar32>(IOutputStream& o, wchar32 ch) {
    size_t length;
    unsigned char buffer[MAX_UTF8_BYTES];
    WriteUTF8Char(ch, length, buffer);
    o.Write(buffer, length);
}

static void WriteString(IOutputStream& o, const wchar16* w, size_t n) {
    const size_t buflen = (n * MAX_UTF8_BYTES); // * 4 because the conversion functions can convert unicode character into maximum 4 bytes of UTF8
    TTempBuf buffer(buflen + 1);
    char* const data = buffer.Data();
    size_t written = 0;
    WideToUTF8(w, n, data, written);
    data[written] = 0;
    o.Write(data, written);
}

static void WriteString(IOutputStream& o, const wchar32* w, size_t n) {
    const size_t buflen = (n * MAX_UTF8_BYTES); // * 4 because the conversion functions can convert unicode character into maximum 4 bytes of UTF8
    TTempBuf buffer(buflen + 1);
    char* const data = buffer.Data();
    size_t written = 0;
    WideToUTF8(w, n, data, written);
    data[written] = 0;
    o.Write(data, written);
}

template <>
void Out<std::string>(IOutputStream& o, const std::string& p) {
    o.Write(p.data(), p.size());
}

template <>
void Out<std::string_view>(IOutputStream& o, const std::string_view& p) {
    o.Write(p.data(), p.length());
}

template <>
void Out<std::u16string_view>(IOutputStream& o, const std::u16string_view& p) {
    WriteString(o, p.data(), p.length());
}

template <>
void Out<std::u32string_view>(IOutputStream& o, const std::u32string_view& p) {
    WriteString(o, p.data(), p.length());
}

template <>
void Out<std::filesystem::path>(IOutputStream& o, const std::filesystem::path& p) {
    o.Write(p.string());
}

template <>
void Out<const wchar16*>(IOutputStream& o, const wchar16* w) {
    if (w) {
        WriteString(o, w, std::char_traits<wchar16>::length(w));
    } else {
        o.Write("(null)");
    }
}

template <>
void Out<const wchar32*>(IOutputStream& o, const wchar32* w) {
    if (w) {
        WriteString(o, w, std::char_traits<wchar32>::length(w));
    } else {
        o.Write("(null)");
    }
}

template <>
void Out<std::u16string>(IOutputStream& o, const std::u16string& w) {
    WriteString(o, w.c_str(), w.size());
}

template <>
void Out<std::u32string>(IOutputStream& o, const std::u32string& w) {
    WriteString(o, w.c_str(), w.size());
}

#define DEF_CONV_DEFAULT(type)                  \
    template <>                                 \
    void Out<type>(IOutputStream & o, type p) { \
        o << ToString(p);                       \
    }

#define DEF_CONV_CHR(type)                      \
    template <>                                 \
    void Out<type>(IOutputStream & o, type p) { \
        o.Write((char)p);                       \
    }

#define DEF_CONV_NUM(type, len)                                   \
    template <>                                                   \
    void Out<type>(IOutputStream & o, type p) {                   \
        char buf[len];                                            \
        o.Write(buf, ToString(p, buf, sizeof(buf)));              \
    }                                                             \
                                                                  \
    template <>                                                   \
    void Out<volatile type>(IOutputStream & o, volatile type p) { \
        Out<type>(o, p);                                          \
    }

DEF_CONV_NUM(bool, 64)

DEF_CONV_CHR(char)
DEF_CONV_CHR(signed char)
DEF_CONV_CHR(unsigned char)

DEF_CONV_NUM(signed short, 64)
DEF_CONV_NUM(signed int, 64)
DEF_CONV_NUM(signed long int, 64)
DEF_CONV_NUM(signed long long int, 64)

DEF_CONV_NUM(unsigned short, 64)
DEF_CONV_NUM(unsigned int, 64)
DEF_CONV_NUM(unsigned long int, 64)
DEF_CONV_NUM(unsigned long long int, 64)

DEF_CONV_NUM(float, 512)
DEF_CONV_NUM(double, 512)
DEF_CONV_NUM(long double, 512)

#if !defined(_YNDX_LIBCXX_ENABLE_VECTOR_BOOL_COMPRESSION) || (_YNDX_LIBCXX_ENABLE_VECTOR_BOOL_COMPRESSION == 1)
// TODO: acknowledge std::bitset::reference for both libc++ and libstdc++
template <>
void Out<typename std::vector<bool>::reference>(IOutputStream& o, const std::vector<bool>::reference& bit) {
    return Out<bool>(o, static_cast<bool>(bit));
}
#endif

template <>
void Out<const void*>(IOutputStream& o, const void* t) {
    o << Hex(size_t(t));
}

template <>
void Out<void*>(IOutputStream& o, void* t) {
    Out<const void*>(o, t);
}

using TNullPtr = decltype(nullptr);

template <>
void Out<TNullPtr>(IOutputStream& o, TTypeTraits<TNullPtr>::TFuncParam) {
    o << std::string_view("nullptr");
}

#define DEF_OPTIONAL(TYPE)                                                               \
    template <>                                                                          \
    void Out<std::optional<TYPE>>(IOutputStream & o, const std::optional<TYPE>& value) { \
        if (value) {                                                                     \
            o << *value;                                                                 \
        } else {                                                                         \
            o << "(NULL)";                                                               \
        }                                                                                \
    }

DEF_OPTIONAL(ui32);
DEF_OPTIONAL(i64);
DEF_OPTIONAL(ui64);
DEF_OPTIONAL(std::string);
DEF_OPTIONAL(std::string_view);
DEF_OPTIONAL(std::u16string);
DEF_OPTIONAL(std::u16string_view);
DEF_OPTIONAL(double);
DEF_OPTIONAL(unsigned char);
DEF_OPTIONAL(signed char);

#if defined(_android_)
namespace {
    class TAndroidStdIOStreams {
    public:
        TAndroidStdIOStreams()
            : LogLibrary("liblog.so")
            , LogFuncPtr((TLogFuncPtr)LogLibrary.Sym("__android_log_write"))
            , Out(LogFuncPtr)
            , Err(LogFuncPtr)
        {
        }

    public:
        using TLogFuncPtr = void (*)(int, const char*, const char*);

        class TAndroidStdOutput: public IOutputStream {
        public:
            inline TAndroidStdOutput(TLogFuncPtr logFuncPtr) noexcept
                : Buffer()
                , LogFuncPtr(logFuncPtr)
            {
            }

            virtual ~TAndroidStdOutput() {
            }

        private:
            virtual void DoWrite(const void* buf, size_t len) override {
                std::lock_guard guard(BufferMutex);
                Buffer.Write(buf, len);
            }

            virtual void DoFlush() override {
                std::lock_guard guard(BufferMutex);
                LogFuncPtr(ANDROID_LOG_DEBUG, GetTag(), Buffer.Data());
                Buffer.Clear();
            }

            virtual const char* GetTag() const = 0;

        private:
            std::mutex BufferMutex;
            TStringStream Buffer;
            TLogFuncPtr LogFuncPtr;
        };

        class TStdErr: public TAndroidStdOutput {
        public:
            TStdErr(TLogFuncPtr logFuncPtr)
                : TAndroidStdOutput(logFuncPtr)
            {
            }

            virtual ~TStdErr() {
            }

        private:
            virtual const char* GetTag() const override {
                return "stderr";
            }
        };

        class TStdOut: public TAndroidStdOutput {
        public:
            TStdOut(TLogFuncPtr logFuncPtr)
                : TAndroidStdOutput(logFuncPtr)
            {
            }

            virtual ~TStdOut() {
            }

        private:
            virtual const char* GetTag() const override {
                return "stdout";
            }
        };

        static bool Enabled;
        TDynamicLibrary LogLibrary; // field order is important, see constructor
        TLogFuncPtr LogFuncPtr;
        TStdOut Out;
        TStdErr Err;

        static inline TAndroidStdIOStreams& Instance() {
            return *SingletonWithPriority<TAndroidStdIOStreams, 4>();
        }
    };

    bool TAndroidStdIOStreams::Enabled = false;
}
#endif // _android_

namespace {
    class TStdOutput: public IOutputStream {
    public:
        inline TStdOutput(FILE* f) noexcept
            : F_(f)
        {
        }

        ~TStdOutput() override = default;

    private:
        void DoWrite(const void* buf, size_t len) override {
            if (len != fwrite(buf, 1, len, F_)) {
#if defined(_win_)
                // On Windows, if 'F_' is console -- 'fwrite' returns count of written characters.
                // If, for example, console output codepage is UTF-8, then returned value is
                // not equal to 'len'. So, we ignore some 'errno' values...
                if ((errno == 0 || errno == EINVAL || errno == EILSEQ) && _isatty(fileno(F_))) {
                    return;
                }
#endif
                ythrow TSystemError() << "write failed";
            }
        }

        void DoFlush() override {
            if (fflush(F_) != 0) {
                ythrow TSystemError() << "fflush failed";
            }
        }

    private:
        FILE* F_;
    };

    struct TStdIOStreams {
        struct TStdErr: public TStdOutput {
            inline TStdErr()
                : TStdOutput(stderr)
            {
            }

            ~TStdErr() override = default;
        };

        TStdErr Err;

        static inline TStdIOStreams& Instance() {
            return *SingletonWithPriority<TStdIOStreams, 4>();
        }
    };
}

IOutputStream& NPrivate::StdErrStream() noexcept {
#if defined(_android_)
    if (TAndroidStdIOStreams::Enabled) {
        return TAndroidStdIOStreams::Instance().Err;
    }
#endif
    return TStdIOStreams::Instance().Err;
}

void RedirectStdioToAndroidLog(bool redirect) {
#if defined(_android_)
    TAndroidStdIOStreams::Enabled = redirect;
#else
    Y_UNUSED(redirect);
#endif
}
