add_library(cpp-yson-json)

target_link_libraries(cpp-yson-json PUBLIC
  contrib-libs-cxxsupp
  yutil
  library-cpp-json
  cpp-string_builder
)

target_sources(cpp-yson-json PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/yson/json/json_writer.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yson/json/yson2json_adapter.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-yson-json PUBLIC
    contrib-libs-linux-headers
  )
endif()
