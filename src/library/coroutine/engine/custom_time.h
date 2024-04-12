#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/datetime/base.h>
=======
#include <src/util/datetime/base.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

namespace NCoro {
class ITime {
  public:
    virtual TInstant Now() = 0;
};
}
