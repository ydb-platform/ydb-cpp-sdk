#include "stream.h"

#include <string>

namespace NUtils {

size_t ReadLine(IInputStream& in, std::string& st) {
    char ch;

    if (!in.Read(&ch, 1)) {
        return 0;
    }

    st.clear();

    size_t result = 0;
    do {
        ++result;

        if (ch == '\n') {
            break;
        }

        st += ch;
    } while (in.Read(&ch, 1));

    if (result && !st.empty() && st.back() == '\r') {
        st.pop_back();
    }

    return result;
}

}