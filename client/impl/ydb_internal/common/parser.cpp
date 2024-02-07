#include "parser.h"

#include <client/impl/ydb_internal/common/string_helpers.h>
#include <client/ydb_types/exceptions/exceptions.h>

namespace NYdb {

TConnectionInfo ParseConnectionString(const std::string& connectionString) {
    if (connectionString.length() == 0) {
        ythrow TContractViolation("Empty connection string");
    }

    const std::string databaseFlag = "/?database=";
    const std::string grpcProtocol = "grpc://";
    const std::string grpcsProtocol = "grpcs://";
    const std::string localhostDomain = "localhost:";

    TConnectionInfo connectionInfo;
    std::string endpoint;

    size_t pathIndex = connectionString.find(databaseFlag);
    if (pathIndex == std::string::npos){
        pathIndex = connectionString.length();
    }
    if (pathIndex != connectionString.length()) {
        connectionInfo.Database = connectionString.substr(pathIndex + databaseFlag.length());
        endpoint = connectionString.substr(0, pathIndex);
    } else {
        endpoint = connectionString;
    }

    if (!StringStartsWith(endpoint, grpcProtocol) && !StringStartsWith(endpoint, grpcsProtocol) &&
        !StringStartsWith(endpoint, localhostDomain))
    {
        connectionInfo.Endpoint = endpoint;
        connectionInfo.EnableSsl = true;
    } else if (StringStartsWith(endpoint, grpcProtocol)) {
        connectionInfo.Endpoint = endpoint.substr(grpcProtocol.length());
        connectionInfo.EnableSsl = false;
    } else if (StringStartsWith(endpoint, grpcsProtocol)) {
        connectionInfo.Endpoint = endpoint.substr(grpcsProtocol.length());
        connectionInfo.EnableSsl = true;
    } else {
        connectionInfo.Endpoint = endpoint;
        connectionInfo.EnableSsl = false;
    }

    return connectionInfo;
}

} // namespace NYdb

