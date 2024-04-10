add_library(cpp-json-common)

target_link_libraries(cpp-json-common PUBLIC
  yutil
)

target_sources(cpp-json-common PRIVATE
<<<<<<< HEAD:src/library/json/common/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/json/common/defs.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/json/common/defs.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/json/common/CMakeLists.txt
)


