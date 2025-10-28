add_subdirectory(credentials)
add_subdirectory(exceptions)
add_subdirectory(executor)
add_subdirectory(fatal_error_handlers)
add_subdirectory(operation)
add_subdirectory(status)

_ydb_sdk_add_library(client-types)

target_sources(client-types PRIVATE
  ydb.cpp
)

target_link_libraries(client-types PUBLIC
  yutil
  protobuf::libprotobuf
  grpc-client
  yql-public-issue
  enum_serialization_runtime
  impl-internal-common
)

generate_enum_serilization(client-types
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/types/s3_settings.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/types/s3_settings.h
)

generate_enum_serilization(client-types
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/types/status_codes.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/types/status_codes.h
)

_ydb_sdk_make_client_component(Types client-types)
