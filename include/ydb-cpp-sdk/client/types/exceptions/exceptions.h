#pragma once

#include <ydb-cpp-sdk/util/generic/yexception.h>

namespace NYdb {

class TYdbException : public yexception {
public:
    using yexception::yexception;
    TYdbException(const std::string& reason);
};

class TContractViolation : public TYdbException {
public:
    TContractViolation(const std::string& reason);
};

} // namespace NYdb
