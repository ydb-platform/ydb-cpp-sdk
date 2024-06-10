add_ydb_test(NAME util-folder-dirut_ut
  SOURCES
    dirut_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/util/folder
  LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
)

add_ydb_test(NAME util-folder-filelist_ut
  SOURCES
    filelist_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/util/folder
  LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
)

add_ydb_test(NAME util-folder-fts_ut
  SOURCES
    fts_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/util/folder
  LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
    cpp-threading-future
)

add_ydb_test(NAME util-folder-iterator_ut
  SOURCES
    iterator_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/util/folder
  LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
)

add_ydb_test(NAME util-folder-path_ut
  SOURCES
    path_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/util/folder
  LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
)

add_ydb_test(NAME util-folder-pathsplit_ut
  SOURCES
    pathsplit_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/util/folder
  LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
)
