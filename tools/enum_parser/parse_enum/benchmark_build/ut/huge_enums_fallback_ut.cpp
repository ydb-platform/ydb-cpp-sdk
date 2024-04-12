#include <tools/enum_parser/parse_enum/benchmark_build/lib/enum_huge.h_serialized.h>
#include <tools/enum_parser/parse_enum/benchmark_build/lib/enum_enormous.h_serialized.h>
#include <src/library/testing/unittest/registar.h>
#include <src/util/generic/serialized_enum.h>
#include <src/util/string/cast.h>


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
