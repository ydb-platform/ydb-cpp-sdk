#include "reverse.h"

#include <util/charset/wide_specific.h>

#include <algorithm>
#include <vector>

void ReverseInPlace(std::string& string) {
    auto* begin = string.begin();
    std::reverse(begin, begin + string.size());
}

void ReverseInPlace(std::u16string& string) {
    auto* begin = string.begin();
    const auto len = string.size();
    auto* end = begin + string.size();

    std::vector<wchar16> buffer(len);
    wchar16* rbegin = buffer.data() + len;
    for (wchar16* p = begin; p < end;) {
        const size_t symbolSize = W16SymbolSize(p, end);
        rbegin -= symbolSize;
        std::copy(p, p + symbolSize, rbegin);
        p += symbolSize;
    }
    std::copy(buffer.begin(), buffer.end(), begin);
}

void ReverseInPlace(std::u32string& string) {
    auto* begin = string.begin();
    std::reverse(begin, begin + string.size());
}
