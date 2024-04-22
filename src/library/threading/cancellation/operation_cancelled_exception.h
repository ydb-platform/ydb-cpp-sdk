#pragma once

#include <ydb-cpp-sdk/util/generic/yexception.h>

namespace NThreading {

//! The exception to be thrown when an operation has been cancelled
class TOperationCancelledException : public yexception {
};

}
