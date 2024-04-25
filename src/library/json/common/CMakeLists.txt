add_library(cpp-json-common)

target_link_libraries(cpp-json-common PUBLIC
  yutil
)

target_sources(cpp-json-common PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/json/common/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/json/common/defs.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/json/common/defs.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/json/common/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/json/common/defs.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/json/common/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/json/common/defs.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/json/common/defs.cpp
>>>>>>> origin/main
)


