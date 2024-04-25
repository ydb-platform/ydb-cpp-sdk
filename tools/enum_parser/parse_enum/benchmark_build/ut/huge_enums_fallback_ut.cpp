#include <tools/enum_parser/parse_enum/benchmark_build/lib/enum_huge.h_serialized.h>
#include <tools/enum_parser/parse_enum/benchmark_build/lib/enum_enormous.h_serialized.h>
#include <src/library/testing/unittest/registar.h>
#include <src/util/generic/serialized_enum.h>
<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/string/cast.h>
=======
#include <src/util/string/cast.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/string/cast.h>
>>>>>>> origin/main


namespace {
    template <class EEnum>
    void EnumerateEnum() {
        const auto& allValues = GetEnumAllValues<EEnum>();

        TString s;
        for (EEnum e : allValues) {
            UNIT_ASSERT_NO_EXCEPTION(s = ToString(e));
            UNIT_ASSERT_NO_EXCEPTION(e = FromString<EEnum>(s));
        }

        EEnum tmp;
        UNIT_ASSERT_VALUES_EQUAL(TryFromString<EEnum>("", tmp), false);
    }
}

Y_UNIT_TEST_SUITE(TTestHugeEnums) {
    Y_UNIT_TEST(Huge) {
        EnumerateEnum<NHuge::EHuge>();
    }
    Y_UNIT_TEST(Enormous) {
        EnumerateEnum<EEnormous>();
    }
};
