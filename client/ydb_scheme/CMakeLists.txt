add_library(cpp-client-ydb_scheme)

target_link_libraries(cpp-client-ydb_scheme PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  impl-ydb_internal-make_request
  client-ydb_common_client-impl
  cpp-client-ydb_driver
  )

target_sources(cpp-client-ydb_scheme PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_scheme/scheme.cpp
)

generate_enum_serilization(cpp-client-ydb_scheme
  ${CMAKE_SOURCE_DIR}/client/ydb_scheme/scheme.h
  INCLUDE_HEADERS
  client/ydb_scheme/scheme.h
)

add_library(YDB-CPP-SDK::Scheme ALIAS cpp-client-ydb_scheme)
