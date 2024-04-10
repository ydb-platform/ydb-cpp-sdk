add_library(cpp-streams-lzma)

target_link_libraries(cpp-streams-lzma PUBLIC
  yutil
  contrib-libs-lzmasdk
)

target_sources(cpp-streams-lzma PRIVATE
<<<<<<< HEAD:src/library/streams/lzma/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/streams/lzma/lzma.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/streams/lzma/lzma.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/streams/lzma/CMakeLists.txt
)
