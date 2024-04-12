add_library(library-yql-utils)

target_link_libraries(library-yql-utils PUBLIC
  yutil
  cpp-string_utils-quote
  cpp-threading-future
  cpp-deprecated-atomic
)

target_sources(library-yql-utils PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/yql/utils/CMakeLists.txt
  utf8.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/library/yql/utils/utf8.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/library/yql/utils/CMakeLists.txt
=======
  utf8.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)
