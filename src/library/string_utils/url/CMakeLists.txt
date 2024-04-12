add_library(cpp-string_utils-url)

target_link_libraries(cpp-string_utils-url PUBLIC
  yutil
  cpp-string_utils-misc
)

target_sources(cpp-string_utils-url PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/string_utils/url/url.cpp
)


