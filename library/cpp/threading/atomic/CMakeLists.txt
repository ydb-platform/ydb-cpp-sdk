add_library(cpp-threading-atomic)

target_link_libraries(cpp-threading-atomic PUBLIC
  yutil
  cpp-deprecated-atomic
)

target_sources(cpp-threading-atomic PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/threading/atomic/bool.cpp
)
