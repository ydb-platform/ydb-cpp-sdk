#include "string.h"
#include "vector.h"
#include "strbuf.h"

#include <library/cpp/testing/unittest/registar.h>

#if __cplusplus < 202002L
    #include <library/cpp/containers/absl_flat_hash/flat_hash_set.h>
#else
    #include <unordered_set>
#endif

#include <util/str_stl.h>

Y_UNIT_TEST_SUITE(StringHashFunctorTests) {
    Y_UNIT_TEST(TestTransparencyWithUnorderedSet) {
        // Using Abseil hash set because `std::unordered_set` is transparent only from C++20 (while
        // we stuck with C++17 right now).
        using TTransparentHashSet =
    #if __cplusplus < 202002L
            absl::flat_hash_set
    #else
            std::unordered_set
    #endif
            <TString, THash<TString>, TEqualTo<TString>>;

        TTransparentHashSet s = {"foo"};
        // If either `THash` or `TEqualTo` is not transparent compilation will fail.
        UNIT_ASSERT_UNEQUAL(s.find(TStringBuf("foo")), s.end());
        UNIT_ASSERT_EQUAL(s.find(TStringBuf("bar")), s.end());
    }
}
