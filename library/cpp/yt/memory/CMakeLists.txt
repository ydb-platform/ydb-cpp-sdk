add_library(cpp-yt-memory)

target_link_libraries(cpp-yt-memory PUBLIC
  yutil
  cpp-sanitizer-include
  cpp-yt-assert
  cpp-yt-misc
  cpp-yt-malloc
)

target_sources(cpp-yt-memory PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/memory/blob.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/memory/chunked_input_stream.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/memory/chunked_memory_allocator.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/memory/chunked_memory_pool.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/memory/chunked_memory_pool_output.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/memory/chunked_output_stream.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/memory/memory_tag.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/memory/ref.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/memory/ref_tracked.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/memory/safe_memory_reader.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/memory/shared_range.cpp
)

if (NOT WIN32)
  target_compile_options(cpp-yt-memory PRIVATE
    -Wdeprecated-this-capture
  )
endif()
