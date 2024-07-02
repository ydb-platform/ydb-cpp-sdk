add_subdirectory(buffered)
add_subdirectory(json)
add_subdirectory(prometheus)
add_subdirectory(spack)
add_subdirectory(text)

add_library(monlib-encode)
target_link_libraries(monlib-encode PUBLIC
  yutil
  monlib-metrics
  enum_serialization_runtime
  string_utils-misc
)
target_sources(monlib-encode PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/encoder.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/encoder_state.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/format.cpp
)
generate_enum_serilization(monlib-encode
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/encoder_state_enum.h
  GEN_HEADER
  ${YDB_SDK_BINARY_DIR}/src/library/monlib/encode/encoder_state_enum.h_serialized.h
  INCLUDE_HEADERS
  src/library/monlib/encode/encoder_state_enum.h
)

_ydb_sdk_install_targets(TARGETS monlib-encode)
