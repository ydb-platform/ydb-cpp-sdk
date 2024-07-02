add_library(library-logger)
target_link_libraries(library-logger PUBLIC
  yutil
  enum_serialization_runtime
  library-json
  deprecated-atomic
)
target_sources(library-logger PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/backend.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/backend_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/composite.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/element.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/file.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/filter.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/filter_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/log.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/null.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/rotating_file.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/stream.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/sync_page_cache_file.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/system.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/thread.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/thread_creator.cpp
)
generate_enum_serilization(library-logger
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/logger/priority.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/library/logger/priority.h
)

add_global_library_for(library-logger.global library-logger)
target_link_libraries(library-logger.global PUBLIC
  yutil
  enum_serialization_runtime
  library-json
  deprecated-atomic
)
target_sources(library-logger.global PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/composite_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/file_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/null_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/rotating_file_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/stream_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/sync_page_cache_file_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/system_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/uninitialized_creator.cpp
)

_ydb_sdk_install_targets(TARGETS library-logger)
_ydb_sdk_install_targets(TARGETS library-logger.global)
