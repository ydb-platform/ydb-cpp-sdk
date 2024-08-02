if (YDB_SDK_TESTS)
  add_subdirectory(stack)

  add_ydb_test(NAME coroutine-engine-coroutine_ut
    SOURCES
      coroutine_ut.cpp
    LINK_LIBRARIES
      coroutine-engine
      cpp-testing-unittest_main
    LABELS
      unit
  )

  target_compile_options(coroutine-engine-coroutine_ut PRIVATE
    -Wno-deprecated
  )
endif()

_ydb_sdk_add_library(coroutine-engine)

target_link_libraries(coroutine-engine
  PUBLIC
    yutil
    containers-intrusive_rb_tree
  PRIVATE
    enum_serialization_runtime
)

target_sources(coroutine-engine
  PRIVATE
    cont_poller.cpp
    helper.cpp
    impl.cpp
    iostatus.cpp
    network.cpp
    poller.cpp
    sockpool.cpp
    stack/stack.cpp
    stack/stack_allocator.cpp
    stack/stack_guards.cpp
    stack/stack_storage.cpp
    stack/stack_utils.cpp
    trampoline.cpp
)

generate_enum_serilization(coroutine-engine
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/engine/poller.h
  INCLUDE_HEADERS
  library/cpp/coroutine/engine/poller.h
)

generate_enum_serilization(coroutine-engine
  ${YDB_SDK_SOURCE_DIR}/library/cpp/coroutine/engine/stack/stack_common.h
  INCLUDE_HEADERS
  library/cpp/coroutine/engine/stack/stack_common.h
)

_ydb_sdk_install_targets(TARGETS coroutine-engine)
