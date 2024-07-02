add_library(library-coroutine-engine)

target_link_libraries(library-coroutine-engine PUBLIC
  yutil
  enum_serialization_runtime
  containers-intrusive_rb_tree
  deprecated-atomic
)

target_sources(library-coroutine-engine PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/cont_poller.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/helper.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/impl.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/iostatus.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/network.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/poller.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/sockpool.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/stack/stack.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/stack/stack_allocator.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/stack/stack_guards.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/stack/stack_storage.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/stack/stack_utils.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/trampoline.cpp
)

generate_enum_serilization(library-coroutine-engine
  ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/poller.h
  INCLUDE_HEADERS
  src/library/coroutine/engine/poller.h
)

generate_enum_serilization(library-coroutine-engine
  ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/engine/stack/stack_common.h
  INCLUDE_HEADERS
  src/library/coroutine/engine/stack/stack_common.h
)

_ydb_sdk_install_targets(TARGETS library-coroutine-engine)
