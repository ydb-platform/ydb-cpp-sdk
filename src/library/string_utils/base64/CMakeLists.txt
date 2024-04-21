add_library(cpp-string_utils-base64)

target_link_libraries(cpp-string_utils-base64 PUBLIC
  yutil
  aklomp::base64
)

target_sources(cpp-string_utils-base64 PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/string_utils/base64/base64.cpp
)


