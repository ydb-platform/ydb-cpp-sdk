if (YDB_SDK_TESTS)
  add_ydb_test(NAME containers-cow_string-medium-ut
    SOURCES
      ut_medium/cow_string_medium_ut.cpp
    LINK_LIBRARIES
      containers-cow_string
      cpp-testing-unittest_main
    LABELS
      medium
  )
  target_include_directories(containers-cow_string-medium-ut PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()

_ydb_sdk_add_library(containers-cow_string)

target_link_libraries(containers-cow_string
  PUBLIC
    yutil
)

target_sources(containers-cow_string
  PRIVATE
    cow_string.cpp
    output.cpp
    reverse.cpp
    subst.cpp
    ysaveload.cpp
)

_ydb_sdk_install_targets(TARGETS containers-cow_string)
