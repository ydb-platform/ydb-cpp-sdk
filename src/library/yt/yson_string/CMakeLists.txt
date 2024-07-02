add_library(library-yt-yson_string)

target_link_libraries(library-yt-yson_string PUBLIC
  yutil
  library-yt-assert
  library-yt-coding
  library-yt-exception
  library-yt-string
  library-yt-memory
  library-yt-misc
)

target_sources(library-yt-yson_string PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/yson_string/convert.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/yson_string/string.cpp
)

if (NOT (WIN32 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64"))
  target_compile_options(library-yt-yson_string PRIVATE
    -Wdeprecated-this-capture
  )
endif()

_ydb_sdk_install_targets(TARGETS library-yt-yson_string)
