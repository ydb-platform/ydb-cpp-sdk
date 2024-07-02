add_library(string_utils-url)

target_link_libraries(string_utils-url PUBLIC
  yutil
  string_utils-misc
)

target_sources(string_utils-url PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/string_utils/url/url.cpp
)

_ydb_sdk_install_targets(TARGETS string_utils-url)
