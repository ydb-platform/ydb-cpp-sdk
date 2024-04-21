#include "gtest.h"

#include <src/library/testing/common/env.h>

std::optional<std::string_view> NGTest::GetTestParam(std::string_view name) {
    auto val = ::GetTestParam(name);
    if (val.empty()) {
        return {};
    } else {
        return {val};
    }
}
