_ydb_sdk_add_library(string_utils-relaxed_escaper)

target_link_libraries(string_utils-relaxed_escaper
  PUBLIC
    yutil
)

target_sources(string_utils-relaxed_escaper
  PRIVATE
    relaxed_escaper.cpp
)

_ydb_sdk_install_targets(TARGETS string_utils-relaxed_escaper)
