#pragma once

#include "odbc_compat.h"

#include <memory>

namespace NYdb::NOdbc {

class TErrorManager;

void RegisterHandle(SQLHANDLE handle, std::shared_ptr<TErrorManager> owner);
std::shared_ptr<TErrorManager> PinHandle(SQLHANDLE handle);
void UnregisterHandle(SQLHANDLE handle);

} // namespace NYdb::NOdbc
