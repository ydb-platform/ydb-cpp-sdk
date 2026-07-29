_ydb_sdk_add_library(yt-assert)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(yt-assert PRIVATE
    -Wdeprecated-this-capture
  )
endif()

target_link_libraries(yt-assert PUBLIC
  yutil
)
target_sources(yt-assert PRIVATE
  assert/assert.cpp
)
_ydb_sdk_install_targets(TARGETS yt-assert)


_ydb_sdk_add_library(yt-coding INTERFACE)
target_link_libraries(yt-coding
  INTERFACE
    yutil
    yt-exception
)
_ydb_sdk_install_targets(TARGETS yt-coding)


_ydb_sdk_add_library(yt-containers INTERFACE)
target_link_libraries(yt-containers
  INTERFACE
    yt-assert
    yt-misc
)
_ydb_sdk_install_targets(TARGETS yt-containers)


_ydb_sdk_add_library(yt-exception)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(yt-exception PRIVATE
    -Wdeprecated-this-capture
  )
endif()

target_link_libraries(yt-exception PUBLIC
  yutil
)
target_sources(yt-exception PRIVATE
  exception/exception.cpp
)
_ydb_sdk_install_targets(TARGETS yt-exception)


_ydb_sdk_add_library(yt-malloc)
target_link_libraries(yt-malloc PUBLIC
  yutil
)
target_sources(yt-malloc PRIVATE
  malloc/malloc.cpp
)
if (NOT (WIN32 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64") AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(yt-malloc PRIVATE
    -Wdeprecated-this-capture
  )
endif()
_ydb_sdk_install_targets(TARGETS yt-malloc)


_ydb_sdk_add_library(yt-memory)
target_link_libraries(yt-memory
  PUBLIC
    yutil
    yt-assert
    yt-misc
    yt-malloc
    yt-system
)
target_sources(yt-memory
  PRIVATE
    memory/allocation_tags_hooks.cpp
    memory/blob.cpp
    memory/chunked_input_stream.cpp
    memory/chunked_memory_allocator.cpp
    memory/chunked_memory_pool.cpp
    memory/chunked_memory_pool_output.cpp
    memory/chunked_output_stream.cpp
    memory/memory_tag.cpp
    memory/new.cpp
    memory/poison.cpp
    memory/ref.cpp
    memory/ref_tracked.cpp
    memory/safe_memory_reader.cpp
    memory/shared_range.cpp
    memory/simple_memory_usage_tracker.cpp
)
if (NOT WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(yt-memory PRIVATE
    -Wdeprecated-this-capture
  )
endif()
_ydb_sdk_install_targets(TARGETS yt-memory)


_ydb_sdk_add_library(yt-misc)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(yt-misc PRIVATE
    -Wdeprecated-this-capture
  )
endif()
target_link_libraries(yt-misc
  PUBLIC
    yutil
    yt-exception
)
target_sources(yt-misc
  PRIVATE
    misc/guid.cpp
    misc/source_location.cpp
)
_ydb_sdk_install_targets(TARGETS yt-misc)


_ydb_sdk_add_library(yt-system)
target_link_libraries(yt-system
  PUBLIC
    yutil
    yt-exception
    yt-misc
)
target_sources(yt-system
  PRIVATE
    system/exit.cpp
    system/thread_name.cpp
)
_ydb_sdk_install_targets(TARGETS yt-system)


_ydb_sdk_add_library(yt-compact_containers INTERFACE)
target_link_libraries(yt-compact_containers
  INTERFACE
    yutil
    yt-assert
    yt-malloc
    yt-misc
)
_ydb_sdk_install_targets(TARGETS yt-compact_containers)


_ydb_sdk_add_library(yt-string)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(yt-string PRIVATE
    -Wdeprecated-this-capture
  )
endif()
target_link_libraries(yt-string 
  PUBLIC
    yutil
    yt-assert
    yt-exception
    yt-misc
    yt-compact_containers
    yt-containers
)
target_sources(yt-string
  PRIVATE
    string/enum.cpp
    string/format_string.cpp
    string/guid.cpp
    string/string.cpp
    string/format.cpp
    string/string_builder.cpp
    string/stream.cpp
)
_ydb_sdk_install_targets(TARGETS yt-string)


_ydb_sdk_add_library(yt-yson)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(yt-yson PRIVATE
    -Wdeprecated-this-capture
  )
endif()
target_link_libraries(yt-yson
  PUBLIC
    yutil
    yt-yson_string
)
target_sources(yt-yson PRIVATE
  yson/consumer.cpp
)
_ydb_sdk_install_targets(TARGETS yt-yson)


_ydb_sdk_add_library(yt-yson_string)
target_link_libraries(yt-yson_string
  PUBLIC
    yutil
    containers-cow_string
    yt-exception
    yt-string
    yt-memory
    yt-misc
  PRIVATE
    yt-assert
    yt-coding
)
target_sources(yt-yson_string PRIVATE
  yson_string/convert.cpp
  yson_string/string.cpp
)
if (NOT (WIN32 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64") AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(yt-yson_string PRIVATE
    -Wdeprecated-this-capture
  )
endif()
_ydb_sdk_install_targets(TARGETS yt-yson_string)
