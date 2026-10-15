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

# Internal headers need SDK compiler settings, but the driver already embeds
# the SDK: linking it again would register SDK globals twice.
foreach(target IN ITEMS odbc-concurrency_it odbc-descriptor_ut)
  target_include_directories(${target} PRIVATE
    $<TARGET_PROPERTY:YDB-CPP-SDK::Query,INTERFACE_INCLUDE_DIRECTORIES>)
  target_compile_options(${target} PRIVATE
    $<TARGET_PROPERTY:YDB-CPP-SDK::Query,INTERFACE_COMPILE_OPTIONS>)
  target_compile_definitions(${target} PRIVATE
    $<TARGET_PROPERTY:YDB-CPP-SDK::Query,INTERFACE_COMPILE_DEFINITIONS>)
endforeach()
