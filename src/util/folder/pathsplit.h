#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/string/ascii.h>
=======
#include <src/util/string/ascii.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/string/ascii.h>
>>>>>>> origin/main

#include <vector>

//do not own any data
struct TPathSplitStore: public std::vector<std::string_view> {
    std::string_view Drive;
    bool IsAbsolute = false;

    void AppendComponent(const std::string_view comp);
    std::string_view Extension() const;

protected:
    std::string DoReconstruct(const std::string_view slash) const;

    inline void DoAppendHint(size_t hint) {
        reserve(size() + hint);
    }
};

struct TPathSplitTraitsUnix: public TPathSplitStore {
    static constexpr char MainPathSep = '/';

    inline std::string Reconstruct() const {
        return DoReconstruct(std::string_view("/"));
    }

    static constexpr bool IsPathSep(const char c) noexcept {
        return c == '/';
    }

    static inline bool IsAbsolutePath(const std::string_view path) noexcept {
        return !path.empty() && IsPathSep(path[0]);
    }

    void DoParseFirstPart(const std::string_view part);
    void DoParsePart(const std::string_view part);
};

struct TPathSplitTraitsWindows: public TPathSplitStore {
    static constexpr char MainPathSep = '\\';

    inline std::string Reconstruct() const {
        return DoReconstruct(std::string_view("\\"));
    }

    static constexpr bool IsPathSep(char c) noexcept {
        return c == '/' || c == '\\';
    }

    static inline bool IsAbsolutePath(const std::string_view path) noexcept {
        return !path.empty() && (IsPathSep(path[0]) || (path.size() > 1 && path[1] == ':' && IsAsciiAlpha(path[0]) && (path.size() == 2 || IsPathSep(path[2]))));
    }

    void DoParseFirstPart(const std::string_view part);
    void DoParsePart(const std::string_view part);
};

#if defined(_unix_)
using TPathSplitTraitsLocal = TPathSplitTraitsUnix;
#else
using TPathSplitTraitsLocal = TPathSplitTraitsWindows;
#endif

template <class TTraits>
class TPathSplitBase: public TTraits {
public:
    inline TPathSplitBase() = default;

    inline TPathSplitBase(const std::string_view part) {
        this->ParseFirstPart(part);
    }

    inline TPathSplitBase& AppendHint(size_t hint) {
        this->DoAppendHint(hint);

        return *this;
    }

    inline TPathSplitBase& ParseFirstPart(const std::string_view part) {
        this->DoParseFirstPart(part);

        return *this;
    }

    inline TPathSplitBase& ParsePart(const std::string_view part) {
        this->DoParsePart(part);

        return *this;
    }

    template <class It>
    inline TPathSplitBase& AppendMany(It b, It e) {
        this->AppendHint(e - b);

        while (b != e) {
            this->AppendComponent(*b++);
        }

        return *this;
    }
};

using TPathSplit = TPathSplitBase<TPathSplitTraitsLocal>;
using TPathSplitUnix = TPathSplitBase<TPathSplitTraitsUnix>;
using TPathSplitWindows = TPathSplitBase<TPathSplitTraitsWindows>;

std::string JoinPaths(const TPathSplit& p1, const TPathSplit& p2);

std::string_view CutExtension(const std::string_view fileName);
