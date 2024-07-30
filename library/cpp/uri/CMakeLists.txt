add_library(uri)

target_link_libraries(uri
  PUBLIC
    yutil
    charset
  PRIVATE
    IDN::IDN
)

target_sources(uri
  PRIVATE
    assign.cpp
    common.cpp
    encode.cpp
    location.cpp
    other.cpp
    parse.cpp
    qargs.cpp
    uri.cpp
)

if (WIN32 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  target_ragel_lexers(uri
    PRIVATE
    ${YDB_SDK_SOURCE_DIR}/library/cpp/uri/encodefsm.rl6
    -CT0
  )
  target_ragel_lexers(uri
    PRIVATE
    ${YDB_SDK_SOURCE_DIR}/library/cpp/uri/parsefsm.rl6
    -CT0
  )
else()
  target_ragel_lexers(uri
    PRIVATE
    ${YDB_SDK_SOURCE_DIR}/library/cpp/uri/encodefsm.rl6
    -CG2
  )
  target_ragel_lexers(uri
    PRIVATE
    ${YDB_SDK_SOURCE_DIR}/library/cpp/uri/parsefsm.rl6
    -CG2
  )
endif()

_ydb_sdk_install_targets(TARGETS uri)
