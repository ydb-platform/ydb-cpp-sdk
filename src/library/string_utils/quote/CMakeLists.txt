add_library(string_utils-quote)

target_link_libraries(string_utils-quote PUBLIC
  yutil
)

target_sources(string_utils-quote PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/string_utils/quote/quote.cpp
)

_ydb_sdk_install_targets(TARGETS string_utils-quote)
