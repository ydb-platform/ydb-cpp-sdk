add_library(cpp-coroutine-listener)

target_link_libraries(cpp-coroutine-listener PUBLIC
  yutil
  cpp-coroutine-engine
)

target_sources(cpp-coroutine-listener PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/library/coroutine/listener/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/listener/listen.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/listener/listen.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/coroutine/listener/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/listener/listen.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/coroutine/listener/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)
