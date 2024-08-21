if (YDB_SDK_TESTS)
  add_ydb_test(NAME http-misc-httpdate_ut
    SOURCES
      httpdate_ut.cpp
    LINK_LIBRARIES
      http-misc
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_test(NAME http-misc-httpreqdata_ut
    SOURCES
      httpreqdata_ut.cpp
    LINK_LIBRARIES
      http-misc
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_test(NAME http-misc-parsed_request_ut
    SOURCES
      parsed_request_ut.cpp
    LINK_LIBRARIES
      http-misc
      cpp-testing-unittest_main
    LABELS
      unit
  )
endif(YDB_SDK_TESTS)

_ydb_sdk_add_library(http-misc)

target_link_libraries(http-misc
  PUBLIC
    yutil
    cgiparam
    digest-lower_case
  PRIVATE
    enum_serialization_runtime
    case_insensitive_string
)

target_sources(http-misc
  PRIVATE
    httpcodes.cpp
    httpdate.cpp
    httpreqdata.cpp
    parsed_request.cpp
)

generate_enum_serilization(http-misc
  ${YDB_SDK_SOURCE_DIR}/library/cpp/http/misc/httpcodes.h
  INCLUDE_HEADERS
  library/cpp/http/misc/httpcodes.h
)

_ydb_sdk_install_targets(TARGETS http-misc)
