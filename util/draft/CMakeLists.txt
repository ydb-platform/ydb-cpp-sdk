if (YDB_SDK_TESTS)
  add_ydb_multiple_tests(PREFIX util-draft
    FILES
      datetime_ut.cpp
      date_ut.cpp
      holder_vector_ut.cpp
      memory_ut.cpp
    ADD_YDB_TEST_ARGS
    LINK_LIBRARIES
      util-draft
      cpp-testing-unittest_main
    LABELS
      unit
  )
endif(YDB_SDK_TESTS)

_ydb_sdk_add_library(util-draft)

target_sources(util-draft PRIVATE
  ${YDB_SDK_SOURCE_DIR}/util/draft/date.cpp
  ${YDB_SDK_SOURCE_DIR}/util/draft/datetime.cpp
  ${YDB_SDK_SOURCE_DIR}/util/draft/enum.cpp
  ${YDB_SDK_SOURCE_DIR}/util/draft/holder_vector.cpp
  ${YDB_SDK_SOURCE_DIR}/util/draft/ip.cpp
  ${YDB_SDK_SOURCE_DIR}/util/draft/matrix.cpp
  ${YDB_SDK_SOURCE_DIR}/util/draft/memory.cpp
)
