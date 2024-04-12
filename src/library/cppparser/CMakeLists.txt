add_library(library-cpp-cppparser)

target_link_libraries(library-cpp-cppparser PUBLIC
  yutil
)

target_sources(library-cpp-cppparser PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/cppparser/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/cppparser/parser.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/cppparser/parser.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/cppparser/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/cppparser/parser.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)


