add_library(cpp-yt-malloc)

target_link_libraries(cpp-yt-malloc PUBLIC
  yutil
)

target_sources(cpp-yt-malloc PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/malloc/malloc.cpp
)

if (NOT (WIN32 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64"))
  target_compile_options(cpp-yt-malloc PRIVATE
    -Wdeprecated-this-capture
  )
endif ()
