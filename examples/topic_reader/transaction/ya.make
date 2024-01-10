PROGRAM(read_from_topic_in_transaction)

SRCS(
    application.cpp
    main.cpp
    options.cpp
)

PEERDIR(
    client/ydb_topic
    library/cpp/getopt
)

END()
