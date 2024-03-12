add_subdirectory(common)
add_subdirectory(fast_sax)
add_subdirectory(writer)

add_library(library-cpp-json)

target_link_libraries(library-cpp-json PUBLIC
  yutil
  contrib-libs-rapidjson
  cpp-json-common
  cpp-json-fast_sax
  cpp-json-writer
  cpp-string_utils-relaxed_escaper
)

target_sources(library-cpp-json PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/json/json_writer.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/json/json_reader.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/json/json_prettifier.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/json/rapidjson_helpers.cpp
)


if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(library-cpp-json PUBLIC
    
  )
endif()
