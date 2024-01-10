LIBRARY()

SRCS(
    ydb_resources.cpp
    ydb_ca.cpp
)

RESOURCE(
    client/resources/ydb_sdk_version.txt ydb_sdk_version.txt
    client/resources/ydb_root_ca.pem ydb_root_ca.pem
)

END()
