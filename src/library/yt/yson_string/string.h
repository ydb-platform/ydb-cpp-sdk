#pragma once

#include <ydb-cpp-sdk/library/yt/yson_string/public.h>

#include <src/library/yt/memory/ref.h>

#include <variant>

namespace NYT::NYson {

////////////////////////////////////////////////////////////////////////////////

//! Contains a sequence of bytes in YSON encoding annotated with EYsonType describing
//! the content. Could be null. Non-owning.
class TYsonStringBuf
{
public:
    //! Constructs a null instance.
    TYsonStringBuf();

    //! Constructs an instance from TYsonString.
    TYsonStringBuf(const TYsonString& ysonString);

    //! Constructs a non-null instance with given type and content.
    explicit TYsonStringBuf(
        const std::string& data,
        EYsonType type = EYsonType::Node);

    //! Constructs a non-null instance with given type and content.
    explicit TYsonStringBuf(
        std::string_view data,
        EYsonType type = EYsonType::Node);

    //! Constructs a non-null instance with given type and content
    //! (without this overload there is no way to construct TYsonStringBuf from
    //! string literal).
    explicit TYsonStringBuf(
        const char* data,
        EYsonType type = EYsonType::Node);

    //! Returns |true| if the instance is not null.
    explicit operator bool() const;

    //! Returns the underlying YSON bytes. The instance must be non-null.
    std::string_view AsStringBuf() const;

    //! Returns type of YSON contained here. The instance must be non-null.
    EYsonType GetType() const;

protected:
    std::string_view Data_;
    EYsonType Type_;
    bool Null_;
};

////////////////////////////////////////////////////////////////////////////////

//! An owning version of TYsonStringBuf.
/*!
 *  Internally captures the data either via std::string or a polymorphic ref-counted holder.
 */
class TYsonString
{
public:
    //! Constructs a null instance.
    TYsonString();

    //! Constructs an instance from TYsonStringBuf.
    //! Copies the data into a ref-counted payload.
    explicit TYsonString(const TYsonStringBuf& ysonStringBuf);

    //! Constructs an instance from std::string_view.
    //! Copies the data into a ref-counted payload.
    explicit TYsonString(
        std::string_view data,
        EYsonType type = EYsonType::Node);

    //! Constructs an instance from std::string.
    //! Zero-copy for CoW std::string: retains the reference to std::string in payload.
    explicit TYsonString(
        const std::string& data,
        EYsonType type = EYsonType::Node);

    //! Constructs an instance from TSharedRef.
    //! Zero-copy; retains the reference to TSharedRef holder in payload.
    explicit TYsonString(
        const TSharedRef& ref,
        EYsonType type = EYsonType::Node);

    //! Returns |true| if the instance is not null.
    explicit operator bool() const;

    //! Returns type of YSON contained here. The instance must be non-null.
    EYsonType GetType() const;

    //! Returns the non-owning data. The instance must be non-null.
    std::string_view AsStringBuf() const;

    //! Returns the data represented by std::string. The instance must be non-null.
    //! Copies the data in case the payload is not std::string.
    std::string ToString() const;

    //! Returns the data represented by TSharedRef. The instance must be non-null.
    //! The data is never copied.
    TSharedRef ToSharedRef() const;

    //! Computes the hash code.
    size_t ComputeHash() const;

    //! Allow to serialize/deserialize using the ::Save ::Load functions. See src/util/ysaveload.h.
    void Save(IOutputStream* s) const;
    void Load(IInputStream* s);

private:
    struct TNullPayload
    { };

    std::variant<TNullPayload, TSharedRangeHolderPtr, std::string> Payload_;

    const char* Begin_;
    ui64 Size_ : 56;
    EYsonType Type_ : 8;
};

////////////////////////////////////////////////////////////////////////////////

bool operator == (const TYsonString& lhs, const TYsonString& rhs);
bool operator == (const TYsonString& lhs, const TYsonStringBuf& rhs);
bool operator == (const TYsonStringBuf& lhs, const TYsonString& rhs);
bool operator == (const TYsonStringBuf& lhs, const TYsonStringBuf& rhs);

bool operator != (const TYsonString& lhs, const TYsonString& rhs);
bool operator != (const TYsonString& lhs, const TYsonStringBuf& rhs);
bool operator != (const TYsonStringBuf& lhs, const TYsonString& rhs);
bool operator != (const TYsonStringBuf& lhs, const TYsonStringBuf& rhs);

std::string ToString(const TYsonString& yson);
std::string ToString(const TYsonStringBuf& yson);

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NYson

#define STRING_INL_H_
#include "string-inl.h"
#undef STRING_INL_H_
