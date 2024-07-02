add_library(colorizer)

target_link_libraries(colorizer PUBLIC
  yutil
)

target_sources(colorizer PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/colorizer/colors.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/colorizer/output.cpp
)

_ydb_sdk_install_targets(TARGETS colorizer)
