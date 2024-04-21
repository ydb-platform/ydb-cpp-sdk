add_library(cpp-json-fast_sax)

target_link_libraries(cpp-json-fast_sax PUBLIC
  yutil
  cpp-json-common
)

target_sources(cpp-json-fast_sax PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/json/fast_sax/unescape.cpp
)

if (WIN32 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  target_ragel_lexers(cpp-json-fast_sax
    PRIVATE
    ${YDB_SDK_SOURCE_DIR}/src/library/json/fast_sax/parser.rl6
    -CT0
  )
else ()
  target_ragel_lexers(cpp-json-fast_sax
    PRIVATE
    ${YDB_SDK_SOURCE_DIR}/src/library/json/fast_sax/parser.rl6
    -CG2
  )
endif ()
