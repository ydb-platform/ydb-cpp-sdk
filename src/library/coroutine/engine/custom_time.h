#pragma once

#include <ydb-cpp-sdk/util/datetime/base.h>

namespace NCoro {
class ITime {
  public:
    virtual TInstant Now() = 0;
};
}
