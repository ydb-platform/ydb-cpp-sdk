LIBRARY()

PEERDIR(
    client/iam
    library/cpp/digest/crc32c
    client/ydb_types/credentials
)

SRCS(
    util.cpp
    ydb_credentials_provider_factory.cpp
)

END()

RECURSE_FOR_TESTS(
    ut
)
