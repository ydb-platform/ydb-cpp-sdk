#include "wide.h"

bool CanBeEncoded(std::u16string_view text, ECharset encoding) {
    const size_t LEN = 16;
    const size_t BUFSIZE = LEN * 4;
    char encodeBuf[BUFSIZE];
    wchar16 decodeBuf[BUFSIZE];

    while (!text.empty()) {
        std::u16string_view src = text;
        src.remove_prefix(LEN);
        std::string_view encoded = NDetail::NBaseOps::Recode(src, encodeBuf, encoding);
        std::u16string_view decoded = NDetail::NBaseOps::Recode(encoded, decodeBuf, encoding);
        if (decoded != src)
            return false;
    }

    return true;
}
