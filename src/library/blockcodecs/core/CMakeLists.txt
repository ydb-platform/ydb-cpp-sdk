add_library(cpp-blockcodecs-core)

target_link_libraries(cpp-blockcodecs-core PUBLIC
  yutil
)

target_sources(cpp-blockcodecs-core PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/blockcodecs/core/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/core/codecs.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/core/stream.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/core/codecs.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/core/stream.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/blockcodecs/core/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/core/codecs.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/core/stream.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/blockcodecs/core/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/core/codecs.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/blockcodecs/core/stream.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)


