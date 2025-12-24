if (YDB_SDK_TESTS)
  add_subdirectory(ut)
endif()

_ydb_sdk_add_library(dns)

target_sources(dns
  PRIVATE
    cache.cpp
    thread.cpp
    magic.cpp
)

target_link_libraries(dns
  PUBLIC
    yutil
)

_ydb_sdk_install_targets(TARGETS dns)
