#ifndef ENUM_INL_H_
#error "Direct inclusion of this file is not allowed, include enum.h"
// For the sake of sane code completion.
#include "enum.h"
#endif

#include "format.h"

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/library/string_utils/misc/misc.h>
#include <ydb-cpp-sdk/library/yt/exception/exception.h>
=======
#include <src/library/string_utils/misc/misc.h>
#include <src/library/yt/exception/exception.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/library/string_utils/misc/misc.h>
#include <ydb-cpp-sdk/library/yt/exception/exception.h>
>>>>>>> origin/main

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

namespace NDetail {

[[noreturn]]
void ThrowMalformedEnumValueException(
    std::string_view typeName,
    std::string_view value);

void FormatUnknownEnumValue(
    TStringBuilderBase* builder,
    std::string_view name,
    i64 value);

} // namespace NDetail

template <class T>
std::optional<T> TryParseEnum(std::string_view value)
{
    auto tryFromString = [] (std::string_view value) -> std::optional<T> {
        if (auto decodedValue = TryDecodeEnumValue(value)) {
            auto enumValue = TEnumTraits<T>::FindValueByLiteral(*decodedValue);
            return enumValue ? enumValue : TEnumTraits<T>::FindValueByLiteral(value);
        }

        auto reportError = [value] () {
            throw TSimpleException(Format("Enum value %Qv is neither in a proper underscore case nor in a format \"%v(123)\"",
                value,
                TEnumTraits<T>::GetTypeName()));
        };

        std::string_view typeName;
        auto isTypeNameCorrect = NUtils::NextTok(value, typeName, '(') && typeName == TEnumTraits<T>::GetTypeName();
        if (!isTypeNameCorrect) {
            reportError();
        }

        std::string_view enumValue;
        std::underlying_type_t<T> underlyingValue = 0;
        auto isEnumValueCorrect = NUtils::NextTok(value, enumValue, ')') && TryFromString(enumValue, underlyingValue);
        if (!isEnumValueCorrect) {
            reportError();
        }

        auto isParsingComplete = value.empty();
        if (!isParsingComplete) {
            reportError();
        }

        return static_cast<T>(underlyingValue);
    };

    if constexpr (TEnumTraits<T>::IsBitEnum) {
        T result{};
        std::string_view token;
        while (NUtils::NextTok(value, token, '|')) {
            if (auto scalar = tryFromString(StripString(token))) {
                result |= *scalar;
            } else {
                return {};
            }
        }
        return result;
    } else {
        return tryFromString(value);
    }
}

template <class T>
T ParseEnum(std::string_view value)
{
    if (auto optionalResult = TryParseEnum<T>(value)) {
        return *optionalResult;
    }
    NYT::NDetail::ThrowMalformedEnumValueException(TEnumTraits<T>::GetTypeName(), value);
}

template <class T>
void FormatEnum(TStringBuilderBase* builder, T value, bool lowerCase)
{
    auto formatScalarValue = [builder, lowerCase] (T value) {
        auto optionalLiteral = TEnumTraits<T>::FindLiteralByValue(value);
        if (!optionalLiteral) {
            NYT::NDetail::FormatUnknownEnumValue(
                builder,
                TEnumTraits<T>::GetTypeName(),
                ToUnderlying(value));
            return;
        }

        if (lowerCase) {
            CamelCaseToUnderscoreCase(builder, *optionalLiteral);
        } else {
            builder->AppendString(*optionalLiteral);
        }
    };

    if constexpr (TEnumTraits<T>::IsBitEnum) {
        if (TEnumTraits<T>::FindLiteralByValue(value)) {
            formatScalarValue(value);
            return;
        }
        auto first = true;
        for (auto scalarValue : TEnumTraits<T>::GetDomainValues()) {
            if (Any(value & scalarValue)) {
                if (!first) {
                    builder->AppendString(std::string_view(" | "));
                }
                first = false;
                formatScalarValue(scalarValue);
            }
        }
    } else {
        formatScalarValue(value);
    }
}

template <class T>
std::string FormatEnum(T value)
{
    TStringBuilder builder;
    FormatEnum(&builder, value, /*lowerCase*/ true);
    return builder.Flush();
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
