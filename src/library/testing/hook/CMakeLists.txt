add_library(cpp-testing-hook)

target_link_libraries(cpp-testing-hook PUBLIC
  yutil
)

target_sources(cpp-testing-hook PRIVATE
<<<<<<< HEAD:src/library/testing/hook/CMakeLists.txt
  hook.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/testing/hook/hook.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/testing/hook/CMakeLists.txt
)
