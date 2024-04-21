add_library(cpp-yt-exception)

target_compile_options(cpp-yt-exception PRIVATE
  -Wdeprecated-this-capture
)

target_link_libraries(cpp-yt-exception PUBLIC
  yutil
)

target_sources(cpp-yt-exception PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/exception/exception.cpp
)


