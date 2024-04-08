add_subdirectory(common)

add_library(cpp-client-iam)

target_link_libraries(cpp-client-iam PUBLIC
  yutil
  client-yc_public-iam
  client-iam-common
)

target_sources(cpp-client-iam PRIVATE
  ${YDB_SDK_SOURCE_DIR}/client/iam/iam.cpp
)

add_library(YDB-CPP-SDK::Iam ALIAS cpp-client-iam)
