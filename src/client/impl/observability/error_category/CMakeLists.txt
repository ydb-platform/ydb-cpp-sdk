_ydb_sdk_add_library(impl-observability-error_category)

target_link_libraries(impl-observability-error_category PUBLIC
  yutil
)

target_sources(impl-observability-error_category PRIVATE
  error_category.cpp
)

_ydb_sdk_install_targets(TARGETS impl-observability-error_category)