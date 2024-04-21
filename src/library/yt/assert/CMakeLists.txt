add_library(cpp-yt-assert)

target_compile_options(cpp-yt-assert PRIVATE
  -Wdeprecated-this-capture
)

target_link_libraries(cpp-yt-assert PUBLIC
  
  yutil
)

target_sources(cpp-yt-assert PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/assert/assert.cpp
)


