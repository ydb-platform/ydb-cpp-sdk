add_library(cpp-threading-equeue)

target_link_libraries(cpp-threading-equeue PUBLIC
  yutil
  cpp-deprecated-atomic
)

target_sources(cpp-threading-equeue PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/equeue/equeue.cpp
)
