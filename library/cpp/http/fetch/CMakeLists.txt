if (YDB_SDK_TESTS)
  add_ydb_test(NAME http-fetch-httpfsm_ut
    SOURCES
      httpfsm_ut.cpp
    LINK_LIBRARIES
      http-fetch
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_test(NAME http-fetch-httpparser_ut
    SOURCES
      httpparser_ut.cpp
    LINK_LIBRARIES
      http-fetch
      cpp-testing-unittest_main
    LABELS
      unit
  )
endif(YDB_SDK_TESTS)

_ydb_sdk_add_library(http-fetch)

target_link_libraries(http-fetch
  PUBLIC
    yutil
    http-misc
    logger
    mime-types
    uri
    ZLIB::ZLIB
  PRIVATE
    charset
    digest-md5
    enum_serialization_runtime
)

target_sources(http-fetch PRIVATE
  http_digest.cpp
  http_socket.cpp
  httpheader.cpp
  httpload.cpp
  exthttpcodes.cpp
)

generate_enum_serilization(http-fetch
  ${YDB_SDK_SOURCE_DIR}/library/cpp/http/fetch/httpheader.h
  INCLUDE_HEADERS
  library/cpp/http/fetch/httpheader.h
)

target_ragel_lexers(http-fetch
  PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/http/fetch/httpfsm.rl6
  -CG1
)

_ydb_sdk_install_targets(TARGETS http-fetch)
