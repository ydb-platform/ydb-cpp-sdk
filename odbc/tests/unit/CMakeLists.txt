add_ydb_test(NAME odbc-convert_ut GTEST
  SOURCES
    convert_ut.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/utils/convert.cpp
  INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
  LINK_LIBRARIES
    yutil
    YDB-CPP-SDK::Params
    api-protos
  LABELS
    unit
)

add_ydb_test(NAME odbc-escape_ut GTEST
  SOURCES
    escape_ut.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/utils/escape.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/utils/sql_type_map.cpp
  INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
  LINK_LIBRARIES
    yutil
  LABELS
    unit
)

add_ydb_test(NAME odbc-param_rewrite_ut GTEST
  SOURCES
    param_rewrite_ut.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/utils/param_rewrite.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/utils/sql_type_map.cpp
  INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
  LINK_LIBRARIES
    yutil
  LABELS
    unit
)

add_ydb_test(NAME odbc-conn_string_ut GTEST
  SOURCES
    conn_string_ut.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/utils/util.cpp
  INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
  LINK_LIBRARIES
    yutil
    YDB-CPP-SDK::Params
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
