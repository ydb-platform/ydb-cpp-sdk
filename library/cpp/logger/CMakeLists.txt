add_library(library-cpp-logger)
target_link_libraries(library-cpp-logger PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  library-cpp-json
  cpp-deprecated-atomic
)
target_sources(library-cpp-logger PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/backend.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/backend_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/composite.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/element.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/file.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/filter.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/filter_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/log.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/null.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/rotating_file.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/stream.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/sync_page_cache_file.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/system.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/thread.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/thread_creator.cpp
)
generate_enum_serilization(library-cpp-logger
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/priority.h
  INCLUDE_HEADERS
  library/cpp/logger/priority.h
)

add_global_library_for(library-cpp-logger.global library-cpp-logger)
target_link_libraries(library-cpp-logger.global PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  library-cpp-json
  cpp-deprecated-atomic
)
target_sources(library-cpp-logger.global PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/composite_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/file_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/null_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/rotating_file_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/stream_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/sync_page_cache_file_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/system_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/logger/uninitialized_creator.cpp
)


if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(library-cpp-logger PUBLIC
    
  )
  target_link_libraries(library-cpp-logger.global PUBLIC
    
  )
endif()
