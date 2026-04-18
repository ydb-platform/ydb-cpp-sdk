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

add_ydb_test(NAME odbc-escape_ut GTEST
  SOURCES
    escape_ut.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/utils/escape.cpp
  INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
  LINK_LIBRARIES
    yutil
  LABELS
    unit
)

add_ydb_test(NAME odbc-sql_like_ut GTEST
  SOURCES
    sql_like_ut.cpp
  INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
  LINK_LIBRARIES
    yutil
  LABELS
    unit
)
