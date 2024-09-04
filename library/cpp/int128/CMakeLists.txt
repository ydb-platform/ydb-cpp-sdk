if (YDB_SDK_TESTS)
  add_subdirectory(ut)
endif()

_ydb_sdk_add_library(int128)

target_sources(int128
  PRIVATE
    int128.cpp
)

target_link_libraries(int128
  PUBLIC
    yutil
)
