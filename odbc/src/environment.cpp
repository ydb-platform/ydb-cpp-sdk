#include "environment.h"
#include "connection.h"

namespace NYdb {
namespace NOdbc {

TEnvironment::TEnvironment() : OdbcVersion_(SQL_OV_ODBC3) {}
TEnvironment::~TEnvironment() {}

SQLRETURN TEnvironment::SetAttribute(SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER stringLength) {
    // TODO: реализовать обработку атрибутов
    OdbcVersion_ = attribute == SQL_ATTR_ODBC_VERSION ? reinterpret_cast<intptr_t>(value) : 0;
    return SQL_SUCCESS;
}

} // namespace NOdbc
} // namespace NYdb
