_ydb_sdk_add_library(client-types-exceptions)

target_link_libraries(client-types-exceptions PUBLIC
  yutil
)

target_sources(client-types-exceptions PRIVATE
  exceptions.cpp
)

_ydb_sdk_install_targets(TARGETS client-types-exceptions)
