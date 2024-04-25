#include "lchash.h"
#include "lciter.h"
#include "hash_ops.h"

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/algorithm.h>
=======
#include <src/util/generic/algorithm.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/algorithm.h>
>>>>>>> origin/main

size_t TCIOps::operator()(const std::string_view& s) const noexcept {
    return FnvCaseLess(s, (size_t)0xBEE);
}

size_t TCIOps::operator()(const char* s) const noexcept {
    return operator()(std::string_view(s));
}

bool TCIOps::operator()(const std::string_view& f, const std::string_view& s) const noexcept {
    using TIter = TLowerCaseIterator<const char>;

    return (f.size() == s.size()) && Equal(TIter(f.begin()), TIter(f.end()), TIter(s.begin()));
}

bool TCIOps::operator()(const char* f, const char* s) const noexcept {
    return operator()(std::string_view(f), std::string_view(s));
}
