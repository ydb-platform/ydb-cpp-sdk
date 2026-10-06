set(YDB_ODBC_TEST_CONFIG_DIR "${CMAKE_BINARY_DIR}/odbc")
file(MAKE_DIRECTORY "${YDB_ODBC_TEST_CONFIG_DIR}")
if (CMAKE_CONFIGURATION_TYPES)
  set(YDB_ODBC_TEST_INI "${YDB_ODBC_TEST_CONFIG_DIR}/$<CONFIG>/odbc.ini")
else()
  set(YDB_ODBC_TEST_INI "${YDB_ODBC_TEST_CONFIG_DIR}/odbc.ini")
endif()

set(YDB_ODBC_DSN_SERVER "localhost:2136" CACHE STRING
  "YDB endpoint in odbc.ini generated for ODBC integration tests")
set(YDB_ODBC_DSN_DATABASE "/local" CACHE STRING
  "YDB database path in odbc.ini generated for ODBC integration tests")

file(GENERATE OUTPUT "${YDB_ODBC_TEST_INI}" CONTENT
"[ODBC Data Sources]
YDB=YDB ODBC Driver

[YDB]
Driver=$<TARGET_FILE:ydb-odbc>
Description=YDB Database Connection
Server=${YDB_ODBC_DSN_SERVER}
Database=${YDB_ODBC_DSN_DATABASE}
AuthMode=Anonymous
")

add_subdirectory(integration)
add_subdirectory(unit)
