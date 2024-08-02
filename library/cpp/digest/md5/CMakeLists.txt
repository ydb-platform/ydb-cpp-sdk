if (YDB_SDK_TESTS)
  add_ydb_test(NAME digest-md5_medium_ut
    SOURCES
      md5_medium_ut.cpp
    LINK_LIBRARIES
      yutil
      digest-md5
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_test(NAME digest-md5_ut
    SOURCES
      md5_ut.cpp
    LINK_LIBRARIES
      yutil
      digest-md5
      cpp-testing-unittest_main
    LABELS
      unit
  )
endif()

_ydb_sdk_add_library(digest-md5)

target_link_libraries(digest-md5 
  PUBLIC
    yutil
  PRIVATE
    nayuki_md5
    string_utils-base64
)

target_sources(digest-md5 
  PRIVATE
    md5.cpp
)

_ydb_sdk_install_targets(TARGETS digest-md5)
