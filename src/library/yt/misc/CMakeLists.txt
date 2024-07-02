add_library(library-yt-misc)

target_compile_options(library-yt-misc PRIVATE
  -Wdeprecated-this-capture
)

target_link_libraries(library-yt-misc PUBLIC
  yutil
  library-yt-exception
  enum_serialization_runtime
  string_utils-misc
)

target_sources(library-yt-misc PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/misc/guid.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/misc/source_location.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/misc/thread_name.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/misc/variant.cpp
)

_ydb_sdk_install_targets(TARGETS library-yt-misc)
