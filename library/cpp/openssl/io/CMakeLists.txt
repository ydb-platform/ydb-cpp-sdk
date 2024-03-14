add_library(cpp-openssl-io)

target_link_libraries(cpp-openssl-io PUBLIC
  yutil
  certs
  OpenSSL::SSL
  cpp-openssl-init
  cpp-openssl-method
)

target_sources(cpp-openssl-io PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/openssl/io/stream.cpp
)


