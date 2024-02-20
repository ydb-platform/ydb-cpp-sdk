#include "string_builder.h"

template <>
void Out<NUtils::TYdbStringBuilder>(IOutputStream& os, const NUtils::TYdbStringBuilder& sb) {
    os << static_cast<const std::string&>(sb);
}