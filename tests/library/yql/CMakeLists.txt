add_ydb_test(NAME library-yql-utils-ut
<<<<<<< HEAD
=======
  INCLUDE_DIRS
  ${YDB_SDK_SOURCE_DIR}/src/library/yql/utils
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
  SOURCES
  utf8_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  library-yql-utils
)

add_ydb_test(NAME yql-public-issue-ut
<<<<<<< HEAD
=======
  INCLUDE_DIRS
  ${YDB_SDK_SOURCE_DIR}/src/library/yql/public/issue
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
  SOURCES
  yql_issue_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  yql-public-issue
  cpp-unicode_normalization
)
