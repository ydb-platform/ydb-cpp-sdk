#include "split.h"

template <class TValue>
inline size_t Split(const char* ptr, const char* delim, std::vector<TValue>& values) {
    values.erase(values.begin(), values.end());
    while (ptr && *ptr) {
        ptr += strspn(ptr, delim);
        if (ptr && *ptr) {
            size_t epos = strcspn(ptr, delim);
            assert(epos);
            values.push_back(TValue(ptr, epos));
            ptr += epos;
        }
    }
    return values.size();
}

size_t Split(const char* ptr, const char* delim, std::vector<std::string>& values) {
    return Split<std::string>(ptr, delim, values);
}

size_t Split(const std::string& in, const std::string& delim, std::vector<std::string>& res) {
    return Split(in.data(), delim.data(), res);
}
