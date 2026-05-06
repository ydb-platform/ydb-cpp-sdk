#pragma once

#include "connection.h"

namespace NYdb::NOdbc {

class TInfoProvider {
public:
    static SQLRETURN GetInfo(
        TConnection* conn,
        SQLUSMALLINT infoType,
        SQLPOINTER infoValuePtr,
        SQLSMALLINT bufferLength,
        SQLSMALLINT* stringLengthPtr);
};

} // namespace NYdb::NOdbc
