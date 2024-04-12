add_library(cpp-threading-atomic)

target_link_libraries(cpp-threading-atomic PUBLIC
  yutil
  cpp-deprecated-atomic
)

target_sources(cpp-threading-atomic PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/atomic/bool.cpp
)
