#include "pathsplit.h"

#include <library/cpp/string_utils/helpers/helpers.h>

#include <util/stream/output.h>
#include <util/generic/yexception.h>

template <class T>
static inline size_t ToReserve(const T& t) {
    size_t ret = t.size() + 5;

    for (auto it = t.begin(); it != t.end(); ++it) {
        ret += it->size();
    }

    return ret;
}

void TPathSplitTraitsUnix::DoParseFirstPart(const std::string_view part) {
    if (part == ".") {
        push_back(".");

        return;
    }

    if (IsAbsolutePath(part)) {
        IsAbsolute = true;
    }

    DoParsePart(part);
}

void TPathSplitTraitsUnix::DoParsePart(const std::string_view part0) {
    DoAppendHint(part0.size() / 8);

    std::string_view next(part0);
    std::string_view part;

    while (NUtils::TrySplit(next, part, next, '/')) {
        AppendComponent(part);
    }

    AppendComponent(next);
}

void TPathSplitTraitsWindows::DoParseFirstPart(const std::string_view part0) {
    std::string_view part(part0);

    if (part == ".") {
        push_back(".");

        return;
    }

    if (IsAbsolutePath(part)) {
        IsAbsolute = true;

        if (part.size() > 1 && part[1] == ':') {
            Drive = part.substr(0, 2);
            part = part.substr(2);
        }
    }

    DoParsePart(part);
}

void TPathSplitTraitsWindows::DoParsePart(const std::string_view part0) {
    DoAppendHint(part0.size() / 8);

    size_t pos = 0;
    std::string_view part(part0);

    while (pos < part.size()) {
        while (pos < part.size() && this->IsPathSep(part[pos])) {
            ++pos;
        }

        const char* begin = part.data() + pos;

        while (pos < part.size() && !this->IsPathSep(part[pos])) {
            ++pos;
        }

        AppendComponent(std::string_view(begin, part.data() + pos));
    }
}

std::string TPathSplitStore::DoReconstruct(const std::string_view slash) const {
    std::string r;

    r.reserve(ToReserve(*this));

    if (IsAbsolute) {
        r.AppendNoAlias(Drive);
        r.AppendNoAlias(slash);
    }

    for (auto i = begin(); i != end(); ++i) {
        if (i != begin()) {
            r.AppendNoAlias(slash);
        }

        r.AppendNoAlias(*i);
    }

    return r;
}

void TPathSplitStore::AppendComponent(const std::string_view comp) {
    if (comp.empty() || comp == ".") {
        // ignore
    } else if (comp == ".." && !empty() && back() != "..") {
        pop_back();
    } else {
        // push back first .. also
        push_back(comp);
    }
}

std::string_view TPathSplitStore::Extension() const {
    return size() > 0 ? CutExtension(back()) : std::string_view();
}

template <>
void Out<TPathSplit>(IOutputStream& o, const TPathSplit& ps) {
    o << ps.Reconstruct();
}

std::string JoinPaths(const TPathSplit& p1, const TPathSplit& p2) {
    if (p2.IsAbsolute) {
        ythrow yexception() << "can not join " << p1 << " and " << p2;
    }

    return TPathSplit(p1).AppendMany(p2.begin(), p2.end()).Reconstruct();
}

std::string_view CutExtension(const std::string_view fileName) {
    if (fileName.empty()) {
        return fileName;
    }

    std::string_view name;
    std::string_view extension;
    NUtils::RSplit(fileName, name, extension, '.');
    if (name.empty()) {
        // dot at a start or not found
        return name;
    } else {
        return extension;
    }
}
