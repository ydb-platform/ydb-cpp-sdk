add_library(library-cpp-cgiparam)

target_link_libraries(library-cpp-cgiparam PUBLIC
  yutil
  library-cpp-iterator
  cpp-string_utils-quote
  cpp-string_utils-scan
)

target_sources(library-cpp-cgiparam PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/cgiparam/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/cgiparam/cgiparam.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/cgiparam/cgiparam.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/cgiparam/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/cgiparam/cgiparam.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/cgiparam/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/cgiparam/cgiparam.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/cgiparam/cgiparam.cpp
>>>>>>> origin/main
)
