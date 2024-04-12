add_library(cpp-string_utils-scan)

target_link_libraries(cpp-string_utils-scan PUBLIC
  yutil
)

target_sources(cpp-string_utils-scan PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/string_utils/scan/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/string_utils/scan/scan.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/string_utils/scan/scan.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/string_utils/scan/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/string_utils/scan/scan.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)


