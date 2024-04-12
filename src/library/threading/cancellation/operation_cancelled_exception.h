#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/yexception.h>
=======
#include <src/util/generic/yexception.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

namespace NThreading {

//! The exception to be thrown when an operation has been cancelled
class TOperationCancelledException : public yexception {
};

}
