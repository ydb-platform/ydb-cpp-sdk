add_subdirectory(protos)

add_library(library-login)

target_link_libraries(library-login PUBLIC
  yutil
  jwt-cpp::jwt-cpp
  protobuf::libprotobuf
  cpp-digest-argonish
  library-cpp-json
  cpp-string_utils-base64
  library-login-protos
)

target_sources(library-login PRIVATE
  login.cpp
)

