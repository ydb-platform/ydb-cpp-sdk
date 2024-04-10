add_library(cpp-yt-memory)

target_link_libraries(cpp-yt-memory PUBLIC
  yutil
  cpp-yt-assert
  cpp-yt-misc
  cpp-yt-malloc
)

target_sources(cpp-yt-memory PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/memory/blob.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/memory/chunked_input_stream.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/memory/chunked_memory_allocator.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/memory/chunked_memory_pool.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/memory/chunked_memory_pool_output.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/memory/chunked_output_stream.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/memory/memory_tag.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/memory/ref.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/memory/ref_tracked.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/memory/safe_memory_reader.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/memory/shared_range.cpp
)

if (NOT WIN32)
  target_compile_options(cpp-yt-memory PRIVATE
    -Wdeprecated-this-capture
  )
endif()
