add_library(library-yt-memory)

target_link_libraries(library-yt-memory PUBLIC
  yutil
  library-yt-assert
  library-yt-misc
  library-yt-malloc
)

target_sources(library-yt-memory PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/memory/blob.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/memory/chunked_input_stream.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/memory/chunked_memory_allocator.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/memory/chunked_memory_pool.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/memory/chunked_memory_pool_output.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/memory/chunked_output_stream.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/memory/memory_tag.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/memory/ref.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/memory/ref_tracked.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/memory/safe_memory_reader.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/memory/shared_range.cpp
)

if (NOT WIN32)
  target_compile_options(library-yt-memory PRIVATE
    -Wdeprecated-this-capture
  )
endif()

_ydb_sdk_install_targets(TARGETS library-yt-memory)
