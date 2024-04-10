add_library(cpp-blockcodecs-core)

target_link_libraries(cpp-blockcodecs-core PUBLIC
  yutil
)

target_sources(cpp-blockcodecs-core PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/core/codecs.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/blockcodecs/core/stream.cpp
)


