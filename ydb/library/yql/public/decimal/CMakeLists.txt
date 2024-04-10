add_library(yql-public-decimal)

target_link_libraries(yql-public-decimal PUBLIC
  yutil
)

target_sources(yql-public-decimal PRIVATE
  ${YDB_SDK_SOURCE_DIR}/ydb/library/yql/public/decimal/yql_decimal.cpp
  ${YDB_SDK_SOURCE_DIR}/ydb/library/yql/public/decimal/yql_decimal_serialize.cpp
)


