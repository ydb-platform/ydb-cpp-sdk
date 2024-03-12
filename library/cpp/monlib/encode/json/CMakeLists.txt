add_library(monlib-encode-json)

target_link_libraries(monlib-encode-json PUBLIC
  yutil
  cpp-monlib-encode
  monlib-encode-buffered
  cpp-monlib-exception
  library-cpp-json
  cpp-json-writer
)

target_sources(monlib-encode-json PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/encode/json/json_decoder.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/encode/json/json_encoder.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(monlib-encode-json PUBLIC
    
  )
endif()
