#include <src/library/resource/resource.h>

#include "ydb_ca.h"

namespace NYdb {

std::string GetRootCertificate() {
    return NResource::Find("ydb_root_ca.pem");
}

} // namespace NYdb