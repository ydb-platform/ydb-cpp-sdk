add_library(cpp-coroutine-listener)

target_link_libraries(cpp-coroutine-listener PUBLIC
  yutil
  cpp-coroutine-engine
)

target_sources(cpp-coroutine-listener PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/listener/listen.cpp
)
