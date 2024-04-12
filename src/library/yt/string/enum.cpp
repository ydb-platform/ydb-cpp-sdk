#include "enum.h"

#include "format.h"

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

template <bool failOnError>
std::optional<std::string> DecodeEnumValueImpl(std::string_view value)
{
    auto camelValue = UnderscoreCaseToCamelCase(value);
    auto underscoreValue = CamelCaseToUnderscoreCase(camelValue);
    if (value != underscoreValue) {
        if constexpr (failOnError) {
            throw TSimpleException(Format("Enum value %Qv is not in a proper underscore case; did you mean %Qv?",
                value,
                underscoreValue));
        } else {
            return std::nullopt;
        }
    }
    return camelValue;
}

std::optional<std::string> TryDecodeEnumValue(std::string_view value)
{
    return DecodeEnumValueImpl<false>(value);
}

std::string DecodeEnumValue(std::string_view value)
{
    auto decodedValue = DecodeEnumValueImpl<true>(value);
    YT_VERIFY(decodedValue);
    return *decodedValue;
}

std::string EncodeEnumValue(std::string_view value)
{
    return CamelCaseToUnderscoreCase(value);
}

namespace NDetail {

void ThrowMalformedEnumValueException(std::string_view typeName, std::string_view value)
{
    throw TSimpleException(Format("Error parsing %v value %Qv",
        typeName,
        value));
}

void FormatUnknownEnumValue(TStringBuilderBase* builder, std::string_view name, i64 value)
{
    builder->AppendFormat("%v(%v)", name, value);
}

} // namespace NDetail

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
