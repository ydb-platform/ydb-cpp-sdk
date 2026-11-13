#pragma once

#include <sql.h>
#include <sqlext.h>

// iODBC exposes this generic macro from its public headers. It collides with
// SDK identifiers included by the driver after the ODBC headers.
#ifdef EXPORT
#undef EXPORT
#endif
