_ydb_sdk_add_library(ydb-c-api SHARED)

target_sources(ydb-c-api PRIVATE
  src/driver.cpp
  src/query.cpp
  src/result.cpp
  src/value.cpp
)

target_include_directories(ydb-c-api PUBLIC include)

target_link_libraries(ydb-c-api
  PRIVATE
    yutil
    ydb-cpp-sdk::Query
    ydb-cpp-sdk::Table
    ydb-cpp-sdk::Driver
)

add_library(ydb-cpp-sdk::c-api ALIAS ydb-c-api)
