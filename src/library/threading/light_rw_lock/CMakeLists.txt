add_library(cpp-threading-light_rw_lock)

target_link_libraries(cpp-threading-light_rw_lock PUBLIC
  yutil
)

target_sources(cpp-threading-light_rw_lock PRIVATE
<<<<<<< HEAD:src/library/threading/light_rw_lock/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/light_rw_lock/lightrwlock.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/light_rw_lock/lightrwlock.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/threading/light_rw_lock/CMakeLists.txt
)
