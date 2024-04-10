add_library(cpp-coroutine-engine)

target_link_libraries(cpp-coroutine-engine PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  contrib-libs-libc_compat
  cpp-containers-intrusive_rb_tree
  cpp-deprecated-atomic
)

target_sources(cpp-coroutine-engine PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/engine/cont_poller.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/engine/helper.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/engine/impl.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/engine/iostatus.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/engine/network.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/engine/poller.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/engine/sockpool.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/engine/stack/stack.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/engine/stack/stack_allocator.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/engine/stack/stack_guards.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/engine/stack/stack_storage.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/engine/stack/stack_utils.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/engine/trampoline.cpp
)

generate_enum_serilization(cpp-coroutine-engine
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/engine/poller.h
  INCLUDE_HEADERS
  library/cpp/coroutine/engine/poller.h
)

generate_enum_serilization(cpp-coroutine-engine
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/engine/stack/stack_common.h
  INCLUDE_HEADERS
  library/cpp/coroutine/engine/stack/stack_common.h
)


