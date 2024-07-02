add_library(library-coroutine-listener)

target_link_libraries(library-coroutine-listener PUBLIC
  yutil
  library-coroutine-engine
)

target_sources(library-coroutine-listener PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/coroutine/listener/listen.cpp
)

_ydb_sdk_install_targets(TARGETS library-coroutine-listener)
