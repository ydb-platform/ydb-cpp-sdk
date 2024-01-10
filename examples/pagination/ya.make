PROGRAM()

SRCS(
    main.cpp
    pagination_data.cpp
    pagination.cpp
)

PEERDIR(
    library/cpp/getopt
    client/ydb_table
)

END()
