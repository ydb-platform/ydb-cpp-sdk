add_library(cpp-coroutine-engine)

target_link_libraries(cpp-coroutine-engine PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  contrib-libs-libc_compat
  cpp-containers-intrusive_rb_tree
  cpp-deprecated-atomic
)

target_sources(cpp-coroutine-engine PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/coroutine/engine/cont_poller.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/coroutine/engine/helper.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/coroutine/engine/impl.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/coroutine/engine/iostatus.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/coroutine/engine/network.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/coroutine/engine/poller.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/coroutine/engine/sockpool.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/coroutine/engine/stack/stack.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/coroutine/engine/stack/stack_allocator.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/coroutine/engine/stack/stack_guards.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/coroutine/engine/stack/stack_storage.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/coroutine/engine/stack/stack_utils.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/coroutine/engine/trampoline.cpp
)

generate_enum_serilization(cpp-coroutine-engine
  ${CMAKE_SOURCE_DIR}/library/cpp/coroutine/engine/poller.h
  INCLUDE_HEADERS
  library/cpp/coroutine/engine/poller.h
)

generate_enum_serilization(cpp-coroutine-engine
  ${CMAKE_SOURCE_DIR}/library/cpp/coroutine/engine/stack/stack_common.h
  INCLUDE_HEADERS
  library/cpp/coroutine/engine/stack/stack_common.h
)


