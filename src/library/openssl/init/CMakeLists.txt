add_library(cpp-openssl-init)

target_link_libraries(cpp-openssl-init PUBLIC
  yutil
  OpenSSL::SSL
)

target_sources(cpp-openssl-init PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/init/init.cpp
)
