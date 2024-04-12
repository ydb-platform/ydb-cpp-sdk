#pragma once

#include <src/util/datetime/base.h>

namespace NCoro {
class ITime {
  public:
    virtual TInstant Now() = 0;
};
}
