add_library(library-json-fast_sax)

target_link_libraries(library-json-fast_sax PUBLIC
  yutil
  library-json-common
)

target_sources(library-json-fast_sax PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/json/fast_sax/unescape.cpp
)

if (WIN32 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  target_ragel_lexers(library-json-fast_sax
    PRIVATE
    ${YDB_SDK_SOURCE_DIR}/src/library/json/fast_sax/parser.rl6
    -CT0
  )
else()
  target_ragel_lexers(library-json-fast_sax
    PRIVATE
    ${YDB_SDK_SOURCE_DIR}/src/library/json/fast_sax/parser.rl6
    -CG2
  )
endif()

_ydb_sdk_install_targets(TARGETS library-json-fast_sax)
