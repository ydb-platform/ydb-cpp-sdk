add_library(library-yt-string)

target_compile_options(library-yt-string PRIVATE
  -Wdeprecated-this-capture
)

target_link_libraries(library-yt-string PUBLIC
  yutil
  library-yt-assert
  library-yt-exception
  library-yt-misc
  library-yt-small_containers
  string_utils-misc
)

target_sources(library-yt-string PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/string/enum.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/string/guid.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/string/string.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/string/format.cpp
)

_ydb_sdk_install_targets(TARGETS library-yt-string)
