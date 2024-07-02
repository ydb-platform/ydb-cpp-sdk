add_library(library-uri)

target_link_libraries(library-uri PUBLIC
  yutil
  IDN::IDN
  library-charset
)

target_sources(library-uri PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/uri/assign.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/uri/common.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/uri/encode.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/uri/location.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/uri/other.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/uri/parse.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/uri/qargs.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/uri/uri.cpp
)

if (WIN32 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
    target_ragel_lexers(library-uri
    PRIVATE
    ${YDB_SDK_SOURCE_DIR}/src/library/uri/encodefsm.rl6
    -CT0
  )
    target_ragel_lexers(library-uri
    PRIVATE
    ${YDB_SDK_SOURCE_DIR}/src/library/uri/parsefsm.rl6
    -CT0
  )
else()
  target_ragel_lexers(library-uri
    PRIVATE
    ${YDB_SDK_SOURCE_DIR}/src/library/uri/encodefsm.rl6
    -CG2
  )
  target_ragel_lexers(library-uri
    PRIVATE
    ${YDB_SDK_SOURCE_DIR}/src/library/uri/parsefsm.rl6
    -CG2
  )
endif()

_ydb_sdk_install_targets(TARGETS library-uri)
