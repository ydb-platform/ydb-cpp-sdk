if (YDB_SDK_TESTS)
  add_subdirectory(ut)
endif()

_ydb_sdk_add_library(containers-paged_vector)

target_link_libraries(containers-paged_vector PUBLIC
  yutil
)

target_sources(containers-paged_vector PRIVATE
  paged_vector.cpp
)

_ydb_sdk_install_targets(TARGETS containers-paged_vector)
