if (YDB_SDK_TESTS)
  add_subdirectory(ut)
endif()

_ydb_sdk_add_library(dbg_output)

target_link_libraries(dbg_output
  PUBLIC
    yutil
    colorizer
)

target_sources(dbg_output
  PRIVATE
    dump.cpp
    dumpers.cpp
    engine.cpp
)

_ydb_sdk_install_targets(TARGETS dbg_output)
