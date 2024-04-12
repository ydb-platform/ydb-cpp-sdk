<<<<<<< HEAD
#include <ydb-cpp-sdk/library/resource/resource.h>

#include <ydb-cpp-sdk/client/resources/ydb_ca.h>
=======
#include <src/library/resource/resource.h>

#include "ydb_ca.h"
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NYdb {

std::string GetRootCertificate() {
    return NResource::Find("ydb_root_ca.pem");
}

} // namespace NYdb