#include "format.h"
#include "string.h"

namespace NYT::NDetail {

////////////////////////////////////////////////////////////////////////////////

template <class TValue>
void FormatValueViaSprintf(
    TStringBuilderBase* builder,
    TValue value,
    std::string_view format,
    std::string_view genericSpec)
{
    constexpr int MaxFormatSize = 64;
    constexpr int SmallResultSize = 64;

    auto copyFormat = [] (char* destination, const char* source, int length) {
        int position = 0;
        for (int index = 0; index < length; ++index) {
            if (IsQuotationSpecSymbol(source[index])) {
                continue;
            }
            destination[position] = source[index];
            ++position;
        }
        return destination + position;
    };

    char formatBuf[MaxFormatSize];
    YT_VERIFY(format.length() >= 1 && format.length() <= MaxFormatSize - 2); // one for %, one for \0
    formatBuf[0] = '%';

    if (format.back() == GenericSpecSymbol) {
        char* formatEnd = copyFormat(formatBuf + 1, format.begin(), format.length() - 1);
        ::memcpy(formatEnd, genericSpec.begin(), genericSpec.length());
        formatEnd[genericSpec.length()] = '\0';
    } else {
        char* formatEnd = copyFormat(formatBuf + 1, format.begin(), format.length());
        *formatEnd = '\0';
    }

    char* result = builder->Preallocate(SmallResultSize);
    size_t resultSize = ::snprintf(result, SmallResultSize, formatBuf, value);
    if (resultSize >= SmallResultSize) {
        result = builder->Preallocate(resultSize + 1);
        YT_VERIFY(::snprintf(result, resultSize + 1, formatBuf, value) == static_cast<int>(resultSize));
    }
    builder->Advance(resultSize);
}

#define XX(type) \
    template \
    void FormatValueViaSprintf( \
        TStringBuilderBase* builder, \
        type value, \
        std::string_view format, \
        std::string_view genericSpec);

XX(i32)
XX(ui32)
XX(i64)
XX(ui64)
XX(float)
XX(double)
XX(const void*)

#undef XX

template <class TValue>
void FormatIntValue(
    TStringBuilderBase* builder,
    TValue value,
    std::string_view format,
    std::string_view genericSpec)
{
    if (format == std::string_view("v")) {
        constexpr int MaxResultSize = 64;
        char buffer[MaxResultSize];
        char* end = buffer + MaxResultSize;
        char* start = WriteDecIntToBufferBackwards(end, value);
        builder->AppendString(std::string_view(start, end));
    } else if (format == std::string_view("x") || format == std::string_view("X")) {
        constexpr int MaxResultSize = 64;
        char buffer[MaxResultSize];
        char* end = buffer + MaxResultSize;
        char* start = WriteHexIntToBufferBackwards(end, value, format[0] == 'X');
        builder->AppendString(std::string_view(start, end));
    } else {
        FormatValueViaSprintf(builder, value, format, genericSpec);
    }
}

#define XX(type) \
    template \
    void FormatIntValue( \
        TStringBuilderBase* builder, \
        type value, \
        std::string_view format, \
        std::string_view genericSpec);

XX(i32)
XX(ui32)
XX(i64)
XX(ui64)

#undef XX

void FormatPointerValue(
    TStringBuilderBase* builder,
    const void* value,
    std::string_view format)
{
    static_assert(sizeof(value) == sizeof(ui64));
    if (format == std::string_view("p") || format == std::string_view("v")) {
        builder->AppendString(std::string_view("0x"));
        FormatValue(builder, reinterpret_cast<ui64>(value), std::string_view("x"));
    } else if (format == std::string_view("x") || format == std::string_view("X")) {
        FormatValue(builder, reinterpret_cast<ui64>(value), format);
    } else {
        builder->AppendString("<invalid pointer format>");
    }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NDetail
