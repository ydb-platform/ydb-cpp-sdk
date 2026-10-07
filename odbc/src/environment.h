#pragma once

#include "utils/handle.h"

#include "odbc_compat.h"

namespace NYdb {
namespace NOdbc {

class TEnvironment : public THandle {
private:
    SQLINTEGER OdbcVersion_;

public:
    TEnvironment();
    ~TEnvironment();

    SQLRETURN SetAttribute(SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER stringLength);
    SQLRETURN GetAttribute(SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER bufferLength, SQLINTEGER* stringLengthPtr);

    SQLRETURN EndTran(SQLSMALLINT completionType);
};

} // namespace NOdbc
} // namespace NYdb
