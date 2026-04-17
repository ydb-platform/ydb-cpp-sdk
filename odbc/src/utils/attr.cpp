#include "attr.h"
#include "diag.h"

#include <algorithm>
#include <cstring>

namespace NYdb::NOdbc {

std::string ReadAttributeString(SQLPOINTER value, SQLINTEGER stringLength) {
    const char* const str = static_cast<const char*>(value);
    if (stringLength == SQL_NTS) {
        return std::string(str);
    }
    if (stringLength < 0) {
        return {};
    }
    return std::string(str, static_cast<size_t>(stringLength));
}

SQLRETURN WriteAttributeString(
    const std::string& source,
    SQLPOINTER value,
    SQLINTEGER bufferLength,
    SQLINTEGER* stringLengthPtr,
    TErrorManager& errors) {
    const SQLINTEGER length = static_cast<SQLINTEGER>(source.size());
    if (stringLengthPtr != nullptr) {
        *stringLengthPtr = length;
    }
    if (value == nullptr) {
        return SQL_SUCCESS;
    }
    if (bufferLength <= 0) {
        return Diag::AddInvalidBufferLength(errors);
    }

    auto* dest = static_cast<SQLCHAR*>(value);
    const size_t maxData = static_cast<size_t>(bufferLength - 1);
    const size_t nCopy = std::min(source.size(), maxData);
    if (nCopy > 0) {
        std::memcpy(dest, source.data(), nCopy);
    }
    dest[nCopy] = 0;

    if (length >= bufferLength) {
        return Diag::AddRightTruncated(errors);
    }
    return SQL_SUCCESS;
}

} // namespace NYdb::NOdbc
