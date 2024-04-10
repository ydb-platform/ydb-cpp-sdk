add_library(cpp-yson-json)

target_link_libraries(cpp-yson-json PUBLIC
  yutil
  library-cpp-json
)

target_sources(cpp-yson-json PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yson/json/json_writer.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yson/json/yson2json_adapter.cpp
)


