add_library(library-cpp-dbg_output)

target_link_libraries(library-cpp-dbg_output PUBLIC
  yutil
  library-cpp-colorizer
  cpp-string_utils-misc
)

target_sources(library-cpp-dbg_output PRIVATE
<<<<<<< HEAD:src/library/dbg_output/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/dbg_output/dump.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/dbg_output/dumpers.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/dbg_output/engine.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/dbg_output/dump.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/dbg_output/dumpers.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/dbg_output/engine.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/dbg_output/CMakeLists.txt
)
