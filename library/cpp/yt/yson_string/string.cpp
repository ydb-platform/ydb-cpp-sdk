#include "string.h"

#include <library/cpp/yt/assert/assert.h>

#include <library/cpp/yt/misc/variant.h>

#include <library/cpp/yt/memory/new.h>

namespace NYT::NYson {

////////////////////////////////////////////////////////////////////////////////

TYsonStringBuf::TYsonStringBuf()
{
    Type_ = EYsonType::Node; // fake
    Null_ = true;
}

TYsonStringBuf::TYsonStringBuf(const TYsonString& ysonString)
{
    if (ysonString) {
        Data_ = ysonString.AsStringBuf();
        Type_ = ysonString.GetType();
        Null_ = false;
    } else {
        Type_ = EYsonType::Node; // fake
        Null_ = true;
    }
}

TYsonStringBuf::TYsonStringBuf(const std::string& data, EYsonType type)
    : TYsonStringBuf(std::string_view(data), type)
{ }

TYsonStringBuf::TYsonStringBuf(std::string_view data, EYsonType type)
    : Data_(data)
    , Type_(type)
    , Null_(false)
{ }

TYsonStringBuf::TYsonStringBuf(const char* data, EYsonType type)
    : TYsonStringBuf(std::string_view(data), type)
{ }

TYsonStringBuf::operator bool() const
{
    return !Null_;
}

std::string_view TYsonStringBuf::AsStringBuf() const
{
    YT_VERIFY(*this);
    return Data_;
}

EYsonType TYsonStringBuf::GetType() const
{
    YT_VERIFY(*this);
    return Type_;
}

////////////////////////////////////////////////////////////////////////////////

TYsonString::TYsonString()
{
    Begin_ = nullptr;
    Size_ = 0;
    Type_ = EYsonType::Node; // fake
}

TYsonString::TYsonString(const TYsonStringBuf& ysonStringBuf)
{
    if (ysonStringBuf) {
        auto data = ysonStringBuf.AsStringBuf();
        auto holder = NDetail::TYsonStringHolder::Allocate(data.length());
        ::memcpy(holder->GetData(), data.data(), data.length());
        Begin_ = holder->GetData();
        Size_ = data.size();
        Type_ = ysonStringBuf.GetType();
        Payload_ = std::move(holder);
    } else {
        Begin_ = nullptr;
        Size_ = 0;
        Type_ = EYsonType::Node; // fake
    }
}

TYsonString::TYsonString(
    std::string_view data,
    EYsonType type)
    : TYsonString(TYsonStringBuf(data, type))
{ }

TYsonString::TYsonString(
    const std::string& data,
    EYsonType type)
    : TYsonString(TYsonStringBuf(data, type))
{ }

TYsonString::TYsonString(
    const TSharedRef& data,
    EYsonType type)
{
    Payload_ = data.GetHolder();
    Begin_ = data.Begin();
    Size_ = data.Size();
    Type_ = type;
}

TYsonString::operator bool() const
{
    return !std::holds_alternative<TNullPayload>(Payload_);
}

EYsonType TYsonString::GetType() const
{
    YT_VERIFY(*this);
    return Type_;
}

std::string_view TYsonString::AsStringBuf() const
{
    YT_VERIFY(*this);
    return std::string_view(Begin_, Begin_ + Size_);
}

std::string TYsonString::ToString() const
{
    return Visit(
        Payload_,
        [] (const TNullPayload&) -> std::string {
            YT_ABORT();
        },
        [&] (const TSharedRangeHolderPtr&) {
            return std::string(AsStringBuf());
        },
        [] (const std::string& payload) {
            return payload;
        });
}

TSharedRef TYsonString::ToSharedRef() const
{
    return Visit(
        Payload_,
        [] (const TNullPayload&) -> TSharedRef {
            YT_ABORT();
        },
        [&] (const TSharedRangeHolderPtr& holder) {
            return TSharedRef(Begin_, Size_, holder);
        },
        [] (const std::string& payload) {
            return TSharedRef::FromString(payload);
        });
}

size_t TYsonString::ComputeHash() const
{
    return THash<std::string_view>()(std::string_view(Begin_, Begin_ + Size_));
}

void TYsonString::Save(IOutputStream* s) const
{
    EYsonType type = Type_;
    if (*this) {
        ::SaveMany(s, type, ToSharedRef());
    } else {
        ::SaveMany(s, type, std::string());
    }
}

void TYsonString::Load(IInputStream* s)
{
    EYsonType type;
    std::string data;
    ::LoadMany(s, type, data);
    if (!data.empty()) {
        *this = TYsonString(data, type);
    } else {
        *this = TYsonString();
    }
}

////////////////////////////////////////////////////////////////////////////////

std::string ToString(const TYsonString& yson)
{
    return yson.ToString();
}

std::string ToString(const TYsonStringBuf& yson)
{
    return std::string(yson.AsStringBuf());
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NYson
