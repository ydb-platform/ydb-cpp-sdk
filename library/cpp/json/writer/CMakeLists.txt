if (YDB_SDK_TESTS)
  add_ydb_test(NAME json-writer-ut
    SOURCES
      json_ut.cpp
      json_value_ut.cpp
    LINK_LIBRARIES
      yutil
      json-writer
      cpp-testing-unittest_main
    LABELS
      unit
    )
endif()

add_library(json-writer)
target_link_libraries(json-writer
  PUBLIC
    yutil
    json-common
  PRIVATE
    enum_serialization_runtime
)
target_sources(json-writer
  PRIVATE
    json_value.cpp
    json.cpp
)
generate_enum_serilization(json-writer
  ${YDB_SDK_SOURCE_DIR}/library/cpp/json/writer/json_value.h
  INCLUDE_HEADERS
  library/cpp/json/writer/json_value.h
)
_ydb_sdk_install_targets(TARGETS json-writer)
