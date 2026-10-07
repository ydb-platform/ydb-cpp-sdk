#pragma once

#include "utils/handle.h"

#include "odbc_compat.h"
#include <unordered_set>
#include <vector>

namespace NYdb {
namespace NOdbc {

class TConnection;

class TEnvironment : public THandle {
private:
    SQLINTEGER OdbcVersion_;
    std::unordered_set<TConnection*> Connections_;
    mutable std::mutex ChildrenMutex_; // Connections_, including allocation unwind and deferred destruction.

public:
    TEnvironment();
    ~TEnvironment();

    SQLRETURN SetAttribute(SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER stringLength);
    SQLRETURN GetAttribute(SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER bufferLength, SQLINTEGER* stringLengthPtr);

    void RegisterConnection(TConnection*);
    void UnregisterConnection(TConnection*);
    bool HasChildren() const;
    std::vector<std::shared_ptr<TConnection>> GetConnectionsSnapshot() const;

    SQLRETURN EndTran(SQLSMALLINT completionType);
};

} // namespace NOdbc
} // namespace NYdb
