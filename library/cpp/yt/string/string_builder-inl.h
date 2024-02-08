#ifndef STRING_BUILDER_INL_H_
#error "Direct inclusion of this file is not allowed, include string.h"
// For the sake of sane code completion.
#include "string_builder.h"
#endif

#include <library/cpp/yt/assert/assert.h>

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

inline char* TYdbStringBuilderBase::Preallocate(size_t size)
{
    Reserve(size + GetLength());
    return Current_;
}

inline void TYdbStringBuilderBase::Reserve(size_t size)
{
    if (Y_UNLIKELY(End_ - Begin_ < static_cast<ssize_t>(size))) {
        size_t length = GetLength();
        auto newLength = std::max(size, MinBufferLength);
        DoReserve(newLength);
        Current_ = Begin_ + length;
    }
}

inline size_t TYdbStringBuilderBase::GetLength() const
{
    return Current_ ? Current_ - Begin_ : 0;
}

inline std::string_view TYdbStringBuilderBase::GetBuffer() const
{
    return std::string_view(Begin_, Current_);
}

inline void TYdbStringBuilderBase::Advance(size_t size)
{
    Current_ += size;
    YT_ASSERT(Current_ <= End_);
}

inline void TYdbStringBuilderBase::AppendChar(char ch)
{
    *Preallocate(1) = ch;
    Advance(1);
}

inline void TYdbStringBuilderBase::AppendChar(char ch, int n)
{
    YT_ASSERT(n >= 0);
    if (Y_LIKELY(0 != n)) {
        char* dst = Preallocate(n);
        ::memset(dst, ch, n);
        Advance(n);
    }
}

inline void TYdbStringBuilderBase::AppendString(std::string_view str)
{
    if (Y_LIKELY(str)) {
        char* dst = Preallocate(str.length());
        ::memcpy(dst, str.begin(), str.length());
        Advance(str.length());
    }
}

inline void TYdbStringBuilderBase::AppendString(const char* str)
{
    AppendString(std::string_view(str));
}

inline void TYdbStringBuilderBase::Reset()
{
    Begin_ = Current_ = End_ = nullptr;
    DoReset();
}

template <class... TArgs>
void TYdbStringBuilderBase::AppendFormat(std::string_view format, TArgs&& ... args)
{
    Format(this, format, std::forward<TArgs>(args)...);
}

template <size_t Length, class... TArgs>
void TYdbStringBuilderBase::AppendFormat(const char (&format)[Length], TArgs&& ... args)
{
    Format(this, format, std::forward<TArgs>(args)...);
}

////////////////////////////////////////////////////////////////////////////////

inline std::string TYdbStringBuilder::Flush()
{
    Buffer_.resize(GetLength());
    auto result = std::move(Buffer_);
    Reset();
    return result;
}

inline void TYdbStringBuilder::DoReset()
{
    Buffer_ = {};
}

inline void TYdbStringBuilder::DoReserve(size_t newLength)
{
    Buffer_.ReserveAndResize(newLength);
    auto capacity = Buffer_.capacity();
    Buffer_.ReserveAndResize(capacity);
    Begin_ = &*Buffer_.begin();
    End_ = Begin_ + capacity;
}

////////////////////////////////////////////////////////////////////////////////

inline void FormatValue(TYdbStringBuilderBase* builder, const TYdbStringBuilder& value, std::string_view /*format*/)
{
    builder->AppendString(value.GetBuffer());
}

template <class T>
std::string ToStringViaBuilder(const T& value, std::string_view spec)
{
    TYdbStringBuilder builder;
    FormatValue(&builder, value, spec);
    return builder.Flush();
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
