#pragma once

#include <string>
#include <util/random/random.h>
#include <util/string/hex.h>

namespace NYdb {

class TTraceUtils {
public:
    // Генерация traceparent в формате W3C Trace Context
    static std::string GenerateTraceParent() {
        // Формат: 00-<trace-id>-<span-id>-<flags>
        return "00-" + GenerateRandomHex(32) + "-" + GenerateRandomHex(16) + "-01";
    }

private:
    // Генерация случайной hex-строки заданной длины
    static std::string GenerateRandomHex(size_t length) {
        const size_t byteLength = length / 2;
        std::string bytes(byteLength, "\0");

        for (size_t i = 0; i < byteLength; ++i) {
            bytes[i] = static_cast<char>(RandomNumber<ui8>());
        }

        return HexEncode(bytes.data(), bytes.size());
    }
};
} // namespace NYdb
