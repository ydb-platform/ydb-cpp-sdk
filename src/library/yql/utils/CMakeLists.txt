add_library(library-yql-utils)

target_link_libraries(library-yql-utils PUBLIC
  yutil
  cpp-string_utils-quote
  cpp-threading-future
  cpp-deprecated-atomic
)

target_sources(library-yql-utils PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/yql/utils/CMakeLists.txt
  utf8.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/library/yql/utils/utf8.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/library/yql/utils/CMakeLists.txt
=======
  utf8.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/library/yql/utils/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  utf8.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  utf8.cpp
>>>>>>> origin/main
)
