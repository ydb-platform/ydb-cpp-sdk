#pragma once

#include <util/generic/string.h>

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

// Forward declarations.
class TYdbStringBuilderBase;
class TYdbStringBuilder;
class TDelimitedStringBuilderWrapper;

template <size_t Length, class... TArgs>
void Format(TYdbStringBuilderBase* builder, const char (&format)[Length], TArgs&&... args);
template <class... TArgs>
void Format(TYdbStringBuilderBase* builder, std::string_view format, TArgs&&... args);

////////////////////////////////////////////////////////////////////////////////

//! A simple helper for constructing strings by a sequence of appends.
class TYdbStringBuilderBase
{
public:
    virtual ~TYdbStringBuilderBase() = default;

    char* Preallocate(size_t size);

    void Reserve(size_t size);

    size_t GetLength() const;

    std::string_view GetBuffer() const;

    void Advance(size_t size);

    void AppendChar(char ch);
    void AppendChar(char ch, int n);

    void AppendString(std::string_view str);
    void AppendString(const char* str);

    template <size_t Length, class... TArgs>
    void AppendFormat(const char (&format)[Length], TArgs&&... args);
    template <class... TArgs>
    void AppendFormat(std::string_view format, TArgs&&... args);

    void Reset();

protected:
    char* Begin_ = nullptr;
    char* Current_ = nullptr;
    char* End_ = nullptr;

    virtual void DoReset() = 0;
    virtual void DoReserve(size_t newLength) = 0;

    static constexpr size_t MinBufferLength = 128;
};

////////////////////////////////////////////////////////////////////////////////

class TYdbStringBuilder
    : public TYdbStringBuilderBase
{
public:
    std::string Flush();

protected:
    std::string Buffer_;

    void DoReset() override;
    void DoReserve(size_t size) override;
};

////////////////////////////////////////////////////////////////////////////////

template <class T>
std::string ToStringViaBuilder(const T& value, std::string_view spec = std::string_view("v"));

////////////////////////////////////////////////////////////////////////////////

//! Appends a certain delimiter starting from the second call.
class TDelimitedStringBuilderWrapper
    : private TNonCopyable
{
public:
    TDelimitedStringBuilderWrapper(
        TYdbStringBuilderBase* builder,
        std::string_view delimiter = std::string_view(", "))
        : Builder_(builder)
        , Delimiter_(delimiter)
    { }

    TYdbStringBuilderBase* operator->()
    {
        if (!FirstCall_) {
            Builder_->AppendString(Delimiter_);
        }
        FirstCall_ = false;
        return Builder_;
    }

private:
    TYdbStringBuilderBase* const Builder_;
    const std::string_view Delimiter_;

    bool FirstCall_ = true;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT

#define STRING_BUILDER_INL_H_
#include "string_builder-inl.h"
#undef STRING_BUILDER_INL_H_
