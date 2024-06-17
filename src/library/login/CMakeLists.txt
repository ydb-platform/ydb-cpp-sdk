add_subdirectory(protos)

add_library(library-login)

target_link_libraries(library-login PUBLIC
  yutil
  jwt-cpp::jwt-cpp
  protobuf::libprotobuf
  digest-argonish
  library-json
  string_utils-base64
  library-login-protos
)

target_sources(library-login PRIVATE
  login.cpp
)

_ydb_sdk_install_targets(TARGETS library-login)
