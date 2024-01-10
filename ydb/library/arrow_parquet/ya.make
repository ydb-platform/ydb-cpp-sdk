LIBRARY()

OWNER(g:kikimr)

SRCS(
    result_set_parquet_printer.cpp
)

PEERDIR(
    client/ydb_value
    contrib/libs/apache/arrow
)

END()
