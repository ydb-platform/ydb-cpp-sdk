add_library(library-cpp-cgiparam)

target_link_libraries(library-cpp-cgiparam PUBLIC
  yutil
  library-cpp-iterator
  cpp-string_utils-quote
  cpp-string_utils-scan
)

target_sources(library-cpp-cgiparam PRIVATE
<<<<<<< HEAD:src/library/cgiparam/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/cgiparam/cgiparam.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/cgiparam/cgiparam.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/cgiparam/CMakeLists.txt
)
