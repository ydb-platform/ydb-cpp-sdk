add_library(cpp-digest-murmur)

target_link_libraries(cpp-digest-murmur PUBLIC
  yutil
)

target_sources(cpp-digest-murmur PRIVATE
<<<<<<< HEAD:src/library/digest/murmur/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/murmur/murmur.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/digest/murmur/murmur.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/digest/murmur/CMakeLists.txt
)
