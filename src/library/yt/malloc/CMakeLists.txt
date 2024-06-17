add_library(library-yt-malloc)

target_link_libraries(library-yt-malloc PUBLIC
  yutil
)

target_sources(library-yt-malloc PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/malloc/malloc.cpp
)

if (NOT (WIN32 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64"))
  target_compile_options(library-yt-malloc PRIVATE
    -Wdeprecated-this-capture
  )
endif()

_ydb_sdk_install_targets(TARGETS library-yt-malloc)
