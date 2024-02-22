#ifndef STRING_BUILDER_INL_H_
#error "Direct inclusion of this file is not allowed, include string.h"
// For the sake of sane code completion.
#include "string_builder.h"
#endif

#include <library/cpp/yt/assert/assert.h>

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

inline char* TStringBuilderBase::Preallocate(size_t size)
{
    Reserve(size + GetLength());
    return Current_;
}

inline void TStringBuilderBase::Reserve(size_t size)
{
    if (Y_UNLIKELY(End_ - Begin_ < static_cast<ssize_t>(size))) {
        size_t length = GetLength();
        auto newLength = std::max(size, MinBufferLength);
        DoReserve(newLength);
        Current_ = Begin_ + length;
    }
}

inline size_t TStringBuilderBase::GetLength() const
{
    return Current_ ? Current_ - Begin_ : 0;
}

inline std::string_view TStringBuilderBase::GetBuffer() const
{
    return std::string_view(Begin_, Current_);
}

inline void TStringBuilderBase::Advance(size_t size)
{
    Current_ += size;
    YT_ASSERT(Current_ <= End_);
}

inline void TStringBuilderBase::AppendChar(char ch)
{
    *Preallocate(1) = ch;
    Advance(1);
}

inline void TStringBuilderBase::AppendChar(char ch, int n)
{
    YT_ASSERT(n >= 0);
    if (Y_LIKELY(0 != n)) {
        char* dst = Preallocate(n);
        ::memset(dst, ch, n);
        Advance(n);
    }
}

inline void TStringBuilderBase::AppendString(std::string_view str)
{
    if (Y_LIKELY(!str.empty())) {
        char* dst = Preallocate(str.length());
        ::memcpy(dst, str.begin(), str.length());
        Advance(str.length());
    }
}

inline void TStringBuilderBase::AppendString(const char* str)
{
    AppendString(std::string_view(str));
}

inline void TStringBuilderBase::Reset()
{
    Begin_ = Current_ = End_ = nullptr;
    DoReset();
}

template <class... TArgs>
void TStringBuilderBase::AppendFormat(std::string_view format, TArgs&& ... args)
{
    Format(this, format, std::forward<TArgs>(args)...);
}

template <size_t Length, class... TArgs>
void TStringBuilderBase::AppendFormat(const char (&format)[Length], TArgs&& ... args)
{
    Format(this, format, std::forward<TArgs>(args)...);
}

////////////////////////////////////////////////////////////////////////////////

inline std::string TStringBuilder::Flush()
{
    Buffer_.resize(GetLength());
    auto result = std::move(Buffer_);
    Reset();
    return result;
}

inline void TStringBuilder::DoReset()
{
    Buffer_ = {};
}

inline void TStringBuilder::DoReserve(size_t newLength)
{
    Buffer_.resize(newLength);
    auto capacity = Buffer_.capacity();
    Buffer_.resize(capacity);
    Begin_ = &*Buffer_.begin();
    End_ = Begin_ + capacity;
}

////////////////////////////////////////////////////////////////////////////////

inline void FormatValue(TStringBuilderBase* builder, const TStringBuilder& value, std::string_view /*format*/)
{
    builder->AppendString(value.GetBuffer());
}

template <class T>
std::string ToStringViaBuilder(const T& value, std::string_view spec)
{
    TStringBuilder builder;
    FormatValue(&builder, value, spec);
    return builder.Flush();
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
