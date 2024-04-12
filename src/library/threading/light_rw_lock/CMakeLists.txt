add_library(cpp-threading-light_rw_lock)

target_link_libraries(cpp-threading-light_rw_lock PUBLIC
  yutil
)

target_sources(cpp-threading-light_rw_lock PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/threading/light_rw_lock/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/light_rw_lock/lightrwlock.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/light_rw_lock/lightrwlock.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/threading/light_rw_lock/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/light_rw_lock/lightrwlock.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/threading/light_rw_lock/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/light_rw_lock/lightrwlock.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)
