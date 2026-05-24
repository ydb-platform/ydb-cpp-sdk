add_library(ydb-odbc SHARED
  src/utils/attr.cpp
  src/utils/escape.cpp
  src/utils/cursor.cpp
  src/utils/types.cpp
  src/utils/util.cpp
  src/utils/convert.cpp
  src/utils/error_manager.cpp
  src/odbc_driver.cpp
  src/connection_attr.cpp
  src/connection.cpp
  src/statement_attr.cpp
  src/statement.cpp
  src/environment.cpp
  src/metadata.cpp
)

target_include_directories(ydb-odbc
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${ODBC_INCLUDE_DIRS}
)

target_link_libraries(ydb-odbc
  PRIVATE
    YDB-CPP-SDK::Query
    YDB-CPP-SDK::Table
    YDB-CPP-SDK::Scheme
    YDB-CPP-SDK::Driver
    ODBC::ODBC
    odbcinst
)

set_target_properties(ydb-odbc PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

include(GNUInstallDirs)

file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/odbcinst.ini"
  CONTENT "[YDB]
Description=YDB ODBC Driver
Driver=$<TARGET_FILE:ydb-odbc>
Setup=$<TARGET_FILE:ydb-odbc>
"
)

set(YDB_ODBC_DRIVER_PATH
  "${CMAKE_INSTALL_FULL_LIBDIR}/libydb-odbc${CMAKE_SHARED_LIBRARY_SUFFIX}")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/odbcinst.ini.in"
  "${CMAKE_CURRENT_BINARY_DIR}/ydb-odbc-odbcinst.ini"
  @ONLY
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ydb-odbc-odbcinst.ini"
  DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/odbcinst.d"
  RENAME ydb-odbc.ini
)

install(TARGETS ydb-odbc
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/
  DESTINATION include/ydb-odbc
)

add_subdirectory(examples)
add_subdirectory(tests)
