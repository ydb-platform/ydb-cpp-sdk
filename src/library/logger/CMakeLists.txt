add_library(library-cpp-logger)
target_link_libraries(library-cpp-logger PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  library-cpp-json
  cpp-deprecated-atomic
)
target_sources(library-cpp-logger PRIVATE
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
generate_enum_serilization(library-cpp-logger
<<<<<<< HEAD
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/logger/priority.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/library/logger/priority.h
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/priority.h
  INCLUDE_HEADERS
  src/library/logger/priority.h
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/logger/priority.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/library/logger/priority.h
>>>>>>> origin/main
)

add_global_library_for(library-cpp-logger.global library-cpp-logger)
target_link_libraries(library-cpp-logger.global PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  library-cpp-json
  cpp-deprecated-atomic
)
target_sources(library-cpp-logger.global PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/composite_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/file_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/null_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/rotating_file_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/stream_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/sync_page_cache_file_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/system_creator.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/logger/uninitialized_creator.cpp
)
<<<<<<< HEAD
<<<<<<< HEAD
=======


if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(library-cpp-logger PUBLIC
    
  )
  target_link_libraries(library-cpp-logger.global PUBLIC
    
  )
endif()
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
