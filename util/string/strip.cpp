#include "strip.h"
#include "ascii.h"

#include <util/string/reverse.h>

void CollapseText(const std::string& from, std::string& to, size_t maxLen) {
    Collapse(from, to, maxLen);
    StripInPlace(to);
    if (to.size() >= maxLen) {
        to.erase(maxLen - 5); // " ..."
        ReverseInPlace(to);
        size_t pos = to.find_first_of(" .,;");
        if (pos != std::string::npos && pos < 32) {
            to.erase(0, pos + 1);
        }
        ReverseInPlace(to);
        to.append(" ...");
    }
}
