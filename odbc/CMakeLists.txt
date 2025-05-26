add_library(ydb-odbc SHARED
  src/utils/convert.cpp
  src/odbc_driver.cpp
  src/connection.cpp
  src/statement.cpp
  src/environment.cpp
)

target_include_directories(ydb-odbc
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ODBC_INCLUDE_DIRS}
)

target_link_libraries(ydb-odbc
  PRIVATE
    YDB-CPP-SDK::Query
    YDB-CPP-SDK::Table
    YDB-CPP-SDK::Driver
    ODBC::ODBC
)

set_target_properties(ydb-odbc PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

install(TARGETS ydb-odbc
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/
  DESTINATION include/ydb-odbc
)

add_subdirectory(examples)
add_subdirectory(tests)

include(GNUInstallDirs)

install(FILES 
  odbcinst.ini
  DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/odbcinst.d
  RENAME ydb-odbc.ini
)

install(FILES
  odbc.ini
  DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}
)
