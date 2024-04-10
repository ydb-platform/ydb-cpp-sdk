add_library(cpp-blockcodecs-core)

target_link_libraries(cpp-blockcodecs-core PUBLIC
  yutil
)

target_sources(cpp-blockcodecs-core PRIVATE
<<<<<<< HEAD:src/library/blockcodecs/core/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/core/codecs.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/core/stream.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/core/codecs.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/core/stream.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/blockcodecs/core/CMakeLists.txt
)


