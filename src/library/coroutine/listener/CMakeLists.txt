add_library(cpp-coroutine-listener)

target_link_libraries(cpp-coroutine-listener PUBLIC
  yutil
  cpp-coroutine-engine
)

target_sources(cpp-coroutine-listener PRIVATE
<<<<<<< HEAD:src/library/coroutine/listener/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/listener/listen.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/listener/listen.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/coroutine/listener/CMakeLists.txt
)
