#pragma once

#include "utils/error_manager.h"

#include <sql.h>
#include <sqlext.h>

namespace NYdb {
namespace NOdbc {

class TConnection;

class TEnvironment : public TErrorManager {
private:
    SQLINTEGER OdbcVersion_;

public:
    TEnvironment();
    ~TEnvironment();

    SQLRETURN SetAttribute(SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER stringLength);
};

} // namespace NOdbc
} // namespace NYdb
