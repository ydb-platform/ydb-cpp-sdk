add_library(cpp-openssl-method)

target_link_libraries(cpp-openssl-method PUBLIC
  yutil
  OpenSSL::SSL
  cpp-openssl-holders
)

target_sources(cpp-openssl-method PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/method/io.cpp
)
