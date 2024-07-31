add_library(logger)
target_link_libraries(logger
  PUBLIC
    yutil
    json
  PRIVATE
    enum_serialization_runtime
)
target_sources(logger
  PRIVATE
    backend.cpp
    backend_creator.cpp
    composite.cpp
    element.cpp
    file.cpp
    filter.cpp
    filter_creator.cpp
    log.cpp
    null.cpp
    rotating_file.cpp
    stream.cpp
    sync_page_cache_file.cpp
    system.cpp
    thread.cpp
    thread_creator.cpp
)
generate_enum_serilization(logger
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/priority.h
  INCLUDE_HEADERS
  library/cpp/logger/priority.h
)

add_global_library_for(logger.global logger)
target_link_libraries(logger.global
  PUBLIC
    yutil
    json
  PRIVATE
    enum_serialization_runtime
)
target_sources(logger.global
  PRIVATE
    composite_creator.cpp
    file_creator.cpp
    null_creator.cpp
    rotating_file_creator.cpp
    stream_creator.cpp
    sync_page_cache_file_creator.cpp
    system_creator.cpp
    uninitialized_creator.cpp
)

_ydb_sdk_install_targets(TARGETS logger)
_ydb_sdk_install_targets(TARGETS logger.global)
