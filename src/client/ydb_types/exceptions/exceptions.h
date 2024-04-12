#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/client/types/exceptions/exceptions.h
#include <ydb-cpp-sdk/util/generic/yexception.h>
========
#include <src/util/generic/yexception.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_types/exceptions/exceptions.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_types/exceptions/exceptions.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

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
