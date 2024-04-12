add_library(cpp-string_utils-base64)

target_link_libraries(cpp-string_utils-base64 PUBLIC
  yutil
  aklomp::base64
)

target_sources(cpp-string_utils-base64 PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/string_utils/base64/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/string_utils/base64/base64.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/string_utils/base64/base64.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/string_utils/base64/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/string_utils/base64/base64.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/string_utils/base64/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/string_utils/base64/base64.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)


