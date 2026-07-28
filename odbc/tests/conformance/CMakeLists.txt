include(FetchContent)

set(_odbc_external_test_environment
  "ODBCINI=${YDB_ODBC_TEST_CONFIG_DIR}/odbc.ini"
  "ODBCSYSINI=${YDB_ODBC_TEST_CONFIG_DIR}"
)

# unixODBC-Test is the public ODBC API conformance corpus. Its build files
# require qmake and the interactive ODBCTest host, so compile the headless
# TestFarm programs directly without changing their sources.
FetchContent_Declare(
  unixodbc_test
  SVN_REPOSITORY https://svn.code.sf.net/p/unixodbc-test/code
  SVN_REVISION -r19
  UPDATE_DISCONNECTED ON
)
FetchContent_MakeAvailable(unixodbc_test)

add_library(unixodbc-test-v3-common STATIC
  "${unixodbc_test_SOURCE_DIR}/TestFarm/v3/common/tst.c"
  "${unixodbc_test_SOURCE_DIR}/TestFarm/v3/common/o3util.c"
  "${unixodbc_test_SOURCE_DIR}/TestFarm/v3/common/o3utdesc.c"
  "${unixodbc_test_SOURCE_DIR}/TestFarm/v3/common/uniutil.c"
)
target_include_directories(unixodbc-test-v3-common
  PUBLIC
    "${unixodbc_test_SOURCE_DIR}/TestFarm/common"
    "${unixodbc_test_SOURCE_DIR}/TestFarm/v3/common"
)
target_compile_definitions(unixodbc-test-v3-common
  PUBLIC
    # Removed from modern 64-bit unixODBC headers, but used by this ODBC 3.x
    # corpus as the row-offset-sized integer type.
    SQLROWOFFSET=SQLLEN
)
target_link_libraries(unixodbc-test-v3-common PUBLIC ODBC::ODBC)
set_target_properties(unixodbc-test-v3-common PROPERTIES
  C_STANDARD 11
  C_STANDARD_REQUIRED ON
)
if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(unixodbc-test-v3-common PRIVATE -w)
endif()

function(add_unixodbc_testfarm_test name source)
  set(target_name "unixodbc-test-${name}")
  add_executable(${target_name} "${unixodbc_test_SOURCE_DIR}/${source}")
  target_link_libraries(${target_name}
    PRIVATE
      unixodbc-test-v3-common
      ODBC::ODBC
  )
  set_target_properties(${target_name} PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
  )
  if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${target_name} PRIVATE -w)
  endif()
  add_dependencies(${target_name} ydb-odbc)

  add_test(NAME "unixodbc-test.${name}"
    COMMAND $<TARGET_FILE:${target_name}> ${ARGN}
  )
  set_tests_properties("unixodbc-test.${name}" PROPERTIES
    ENVIRONMENT "${_odbc_external_test_environment}"
    LABELS "integration;odbc;core-conformance;unixodbc-test"
    TIMEOUT 60
  )
endfunction()

# The upstream TestFarm's formal Core directory contains the two allocation
# tests. The remaining programs are its Core-relevant, SQL-dialect-independent
# conformance cases from the historical Other directory.
add_unixodbc_testfarm_test(alloc-handle-env
  "TestFarm/v3/conformance/Core/AllocHandle-ENV/test.c")
add_unixodbc_testfarm_test(alloc-handle-dbc
  "TestFarm/v3/conformance/Core/AllocHandle-DBC/test.c")
add_unixodbc_testfarm_test(connect
  "TestFarm/v3/conformance/Other/connect/connect.c" YDB)
add_unixodbc_testfarm_test(get-functions
  "TestFarm/v3/conformance/Other/getfunc/getfunc.c" YDB)
add_unixodbc_testfarm_test(get-info
  "TestFarm/v3/conformance/Other/getinfo/getinfo.c" YDB)
add_unixodbc_testfarm_test(get-type-info
  "TestFarm/v3/conformance/Other/gettinf/gettinf.c" YDB)
add_unixodbc_testfarm_test(native-sql
  "TestFarm/v3/conformance/Other/nativsql/nativsql.c" YDB)
add_unixodbc_testfarm_test(connection-attributes
  "TestFarm/v3/conformance/Other/getconop/getconop.c" YDB)
add_unixodbc_testfarm_test(attribute-roundtrip
  "TestFarm/v3/conformance/Other/getset/getset.c" YDB)

# SOCI's ODBC backend uses only Core-classified ODBC entry points. Compile its
# upstream common test sources unchanged; the local translation unit supplies
# only the database schema and capability adapter required by SOCI's harness.
set(SOCI_SHARED OFF CACHE BOOL "" FORCE)
set(SOCI_TESTS OFF CACHE BOOL "" FORCE)
set(SOCI_LTO OFF CACHE BOOL "" FORCE)
set(WITH_BOOST OFF CACHE BOOL "" FORCE)
set(SOCI_DB2 OFF CACHE STRING "" FORCE)
set(SOCI_EMPTY OFF CACHE STRING "" FORCE)
set(SOCI_FIREBIRD OFF CACHE STRING "" FORCE)
set(SOCI_MYSQL OFF CACHE STRING "" FORCE)
set(SOCI_ODBC ON CACHE STRING "" FORCE)
set(SOCI_ORACLE OFF CACHE STRING "" FORCE)
set(SOCI_POSTGRESQL OFF CACHE STRING "" FORCE)
set(SOCI_SQLITE3 OFF CACHE STRING "" FORCE)
FetchContent_Declare(
  soci
  GIT_REPOSITORY https://github.com/SOCI/soci.git
  GIT_TAG 4c1578d09ebbe324fbe1f00661146b483de88be2
  UPDATE_DISCONNECTED ON
)
FetchContent_MakeAvailable(soci)

add_executable(soci-odbc-core-tests
  "${soci_SOURCE_DIR}/tests/common/test-boost.cpp"
  "${soci_SOURCE_DIR}/tests/common/test-common.cpp"
  "${soci_SOURCE_DIR}/tests/common/test-connparams.cpp"
  "${soci_SOURCE_DIR}/tests/common/test-custom.cpp"
  "${soci_SOURCE_DIR}/tests/common/test-dynamic.cpp"
  "${soci_SOURCE_DIR}/tests/common/test-lob.cpp"
  "${soci_SOURCE_DIR}/tests/common/test-main.cpp"
  "${soci_SOURCE_DIR}/tests/common/test-manual.cpp"
  "${soci_SOURCE_DIR}/tests/common/test-rowset.cpp"
  "${soci_SOURCE_DIR}/tests/common/test-unicode.cpp"
  soci_odbc_test_context.cpp
)
target_include_directories(soci-odbc-core-tests
  PRIVATE
    "${soci_SOURCE_DIR}/include/private"
    "${soci_SOURCE_DIR}/tests"
)
target_compile_definitions(soci-odbc-core-tests
  PRIVATE
    CATCH_CONFIG_CPP11_NO_SHUFFLE
)
target_compile_features(soci-odbc-core-tests PRIVATE cxx_std_14)
target_link_libraries(soci-odbc-core-tests
  PRIVATE
    SOCI::Core
    SOCI::ODBC
)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(soci-odbc-core-tests PRIVATE -Wno-deprecated-declarations)
endif()
add_dependencies(soci-odbc-core-tests ydb-odbc)

set(_soci_odbc_connection_string
  "Driver=$<TARGET_FILE:ydb-odbc>;Server=${YDB_ODBC_DSN_SERVER};Database=${YDB_ODBC_DSN_DATABASE}"
)
set(_soci_odbc_core_test_cases
  "timegm implementation"
  "Exception on not connected"
  "Basic functionality"
  "Use and into"
  "Repeated and bulk fetch"
  "Indicators"
  "Indicators vector"
  "Use type conversion"
  "Multiple use and into"
  "Use vector"
  "Named parameters"
  "Named parameters with similar names"
  "Use with indicators"
  "Numeric round trip"
  "Bulk fetch with single use"
  "Basic logging support"
  "soci_error is nothrow"
  "NULL with std optional"
  "Connection and reconnection"
  "Connection pool"
  "Query transformation"
  "Query transformation with connection pool"
  "Backend memory leak"
  "Bind memory leak"
  "std::tm timestamp problem with DST"
  "Truncation error"
  "Select without table"
  "String length"
  "Logger"
  "Connection string parsing"
  "connection_parameters::extract_option"
  "connection_parameters::build_string_from_options"
  "Use vector of custom type objects"
  "Into vector of custom type objects"
  "Prepared insert with ORM"
  "Partial match with ORM"
  "Dynamic row binding"
  "Dynamic row binding 2"
  "Dynamic row binding 3"
  "Dynamic binding with type conversions"
  "Dynamic row binding 4"
  "Dynamic binding with rowset"
  "CLOB"
  "CLOB vector"
  "Rowset creation and copying"
  "Rowset iteration"
  "Reading rows from rowset"
  "Reading ints from rowset"
  "Reading strings from rowset"
  "Rowset expected exception"
  "NULL expected exception"
)

# Deliberately not registered: upstream cases which the SOCI harness would
# capability-skip (identity, affected rows, XML, and BLOB), plus cases requiring
# SQL_AUTOCOMMIT_OFF/rollback. Core-level ODBC drivers need not support true
# transactions, so those are not valid tests of a Core-only framework path.

foreach(test_case IN LISTS _soci_odbc_core_test_cases)
  string(MAKE_C_IDENTIFIER "${test_case}" test_id)
  string(TOLOWER "${test_id}" test_id)
  add_test(NAME "soci-odbc.${test_id}"
    COMMAND $<TARGET_FILE:soci-odbc-core-tests>
      "${_soci_odbc_connection_string}"
      "${test_case}"
  )
  set_tests_properties("soci-odbc.${test_id}" PROPERTIES
    ENVIRONMENT "${_odbc_external_test_environment}"
    LABELS "integration;odbc;core-framework;framework-compatibility;soci"
    RESOURCE_LOCK soci_odbc_test_table
    TIMEOUT 120
  )
endforeach()
