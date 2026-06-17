#pragma once

#include "utils/error_manager.h"

#include <sql.h>
#include <sqlext.h>
#include <unordered_set>
#include <vector>

namespace NYdb {
namespace NOdbc {

class TConnection;

class TEnvironment : public TErrorManager {
private:
    SQLINTEGER OdbcVersion_;
    std::unordered_set<TConnection*> Connections_;

public:
    TEnvironment();
    ~TEnvironment();

    SQLRETURN SetAttribute(SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER stringLength);
    SQLRETURN GetAttribute(SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER bufferLength, SQLINTEGER* stringLengthPtr);

    void RegisterConnection(TConnection*);
    void UnregisterConnection(TConnection*);
    std::vector<TConnection*> GetConnectionsSnapshot() const;

    SQLRETURN EndTran(SQLSMALLINT completionType);
};

} // namespace NOdbc
} // namespace NYdb
