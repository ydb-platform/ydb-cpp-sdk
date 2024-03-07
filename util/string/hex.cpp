#include "hex.h"

const char* const Char2DigitTable = ("\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
                                     "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
                                     "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
                                     "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\xff\xff\xff\xff\xff\xff" //0-9
                                     "\xff\x0a\x0b\x0c\x0d\x0e\x0f\xff\xff\xff\xff\xff\xff\xff\xff\xff" //A-Z
                                     "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
                                     "\xff\x0a\x0b\x0c\x0d\x0e\x0f\xff\xff\xff\xff\xff\xff\xff\xff\xff" //a-z
                                     "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
                                     "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
                                     "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
                                     "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
                                     "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
                                     "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
                                     "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
                                     "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
                                     "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff");

char* HexEncode(const void* in, size_t len, char* out) {
    const unsigned char* b = (const unsigned char*)in;
    const unsigned char* e = b + len;

    while (b != e) {
        *out++ = DigitToChar(*b / 16);
        *out++ = DigitToChar(*b++ % 16);
    }

    return out;
}

void* HexDecode(const void* in, size_t len, void* ptr) {
    const char* b = (const char*)in;
    const char* e = b + len;
    Y_ENSURE(!(len & 1), std::string_view("Odd buffer length passed to HexDecode"));

    char* out = (char*)ptr;

    while (b != e) {
        *out++ = (char)String2Byte(b);
        b += 2;
    }

    return out;
}

std::string HexEncode(const void* in, size_t len) {
    std::string ret;

    ret.resize(len << 1);
    HexEncode(in, len, ret.data());

    return ret;
}

std::string HexDecode(const void* in, size_t len) {
    std::string ret;

    ret.resize(len >> 1);
    HexDecode(in, len, ret.data());

    return ret;
}
