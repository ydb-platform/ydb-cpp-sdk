add_library(string_utils-url)

target_link_libraries(string_utils-url
  PUBLIC
    yutil
)

target_sources(string_utils-url PRIVATE
  url.cpp
)

_ydb_sdk_install_targets(TARGETS string_utils-url)
