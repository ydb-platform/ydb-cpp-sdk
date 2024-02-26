#include "util.h"

#include <util/string/printf.h>

#include <library/cpp/digest/crc32c/crc32c.h>
#include <library/cpp/string_builder/string_builder.h>

namespace NKikimr {

std::string MaskTicket(std::string_view token) {
    NUtils::TYdbStringBuilder mask;
    if (token.size() >= 16) {
        mask << token.substr(0, 4);
        mask << "****";
        mask << token.substr(token.size() - 4, 4);
    } else {
        mask << "****";
    }
    mask << " (";
    mask << Sprintf("%08X", Crc32c(token.data(), token.size()));
    mask << ")";
    return mask;
}

std::string MaskTicket(const std::string& token) {
    return MaskTicket(std::string_view(token));
}

} // namespace NKikimr
