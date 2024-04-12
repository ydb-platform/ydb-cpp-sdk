#include "city.h"

#include <src/library/testing/unittest/registar.h>

Y_UNIT_TEST_SUITE(TCityTest) {
    Y_UNIT_TEST(TestTemplatesCompiling) {
        std::string_view s;
        CityHash64(s);
        CityHash64WithSeed(s, 1);
        CityHash64WithSeeds(s, 1, 2);
        CityHash128(s);
        CityHash128WithSeed(s, uint128(1, 2));
        UNIT_ASSERT(s.empty());
    }
}
