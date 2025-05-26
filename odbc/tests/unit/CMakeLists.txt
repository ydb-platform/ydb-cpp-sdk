add_ydb_test(NAME odbc-convert_ut GTEST
  SOURCES
    convert_ut.cpp
  LINK_LIBRARIES
    yutil
    api-protos
    ydb-odbc
  LABELS
    unit
)
