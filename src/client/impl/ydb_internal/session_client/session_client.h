#pragma once

#include <functional>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/datetime/base.h>
=======
#include <src/util/datetime/base.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NYdb {

class TKqpSessionCommon;

class ISessionClient {
public:
    virtual ~ISessionClient() = default;
    virtual void DeleteSession(TKqpSessionCommon* sessionImpl) = 0;
    // TODO: Try to remove from ISessionClient
    virtual bool ReturnSession(TKqpSessionCommon* sessionImpl) = 0;
};

}
