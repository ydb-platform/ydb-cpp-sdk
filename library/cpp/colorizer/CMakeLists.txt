if (YDB_SDK_TESTS)
  add_subdirectory(ut)
endif()

_ydb_sdk_add_library(colorizer)

target_link_libraries(colorizer PUBLIC
  yutil
)

target_sources(colorizer PRIVATE
  colors.cpp
  output.cpp
)

_ydb_sdk_install_targets(TARGETS colorizer)
