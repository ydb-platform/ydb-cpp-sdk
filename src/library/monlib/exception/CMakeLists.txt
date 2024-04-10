add_library(cpp-monlib-exception)

target_link_libraries(cpp-monlib-exception PUBLIC
  yutil
)

target_sources(cpp-monlib-exception PRIVATE
<<<<<<< HEAD:src/library/monlib/exception/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/exception/exception.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/exception/exception.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/exception/CMakeLists.txt
)


