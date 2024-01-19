LIBRARY()

SRCS(
    yql_issue.cpp
    yql_issue_message.cpp
)

PEERDIR(
    contrib/libs/protobuf
    library/cpp/colorizer
    library/cpp/resource
    ydb/public/api/protos
    ydb/library/yql/public/issue/protos
    ydb/library/yql/utils
)

END()

RECURSE(
    protos
)
