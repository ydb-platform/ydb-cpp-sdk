add_library(cpp-client-helpers)

target_link_libraries(cpp-client-helpers PUBLIC
  yutil
  cpp-client-iam
  client-ydb_types-credentials
  public-issue-protos
)

target_sources(cpp-client-helpers PRIVATE
  ${YDB_SDK_SOURCE_DIR}/client/helpers/helpers.cpp
)

add_library(YDB-CPP-SDK::Helpers ALIAS cpp-client-helpers)
