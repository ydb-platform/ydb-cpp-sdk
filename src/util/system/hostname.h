#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/fwd.h>
=======
#include <src/util/generic/fwd.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

const char* GetHostName();
const std::string& HostName();

const char* GetFQDNHostName();
const std::string& FQDNHostName();
bool IsFQDN(const std::string& name);
