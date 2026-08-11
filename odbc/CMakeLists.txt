add_library(ydb-odbc SHARED
  src/utils/attr.cpp
  src/utils/escape.cpp
  src/utils/sql_type_map.cpp
  src/utils/param_rewrite.cpp
  src/utils/type_info_rows.cpp
  src/utils/cursor.cpp
  src/utils/types.cpp
  src/utils/util.cpp
  src/utils/status_util.cpp
  src/utils/convert.cpp
  src/utils/error_manager.cpp
  src/odbc_driver.cpp
  src/connection_attr.cpp
  src/connection_config.cpp
  src/connection.cpp
  src/statement_attr.cpp
  src/statement.cpp
  src/statement_metadata.cpp
  src/environment.cpp
  src/metadata.cpp
  src/descriptor.cpp
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
    YDB-CPP-SDK::Credentials
    YDB-CPP-SDK::Helpers
    YDB-CPP-SDK::Iam
    ODBC::ODBC
    odbcinst
)

set_target_properties(ydb-odbc PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

include(GNUInstallDirs)

if (YDB_SDK_DEPENDENCY_MODE STREQUAL "SYSTEM" AND CMAKE_LIBRARY_ARCHITECTURE)
  set(_ydb_odbc_default_libdir "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}")
  set(_ydb_odbc_default_datadir "/usr/share/ydb-odbc")
else()
  set(_ydb_odbc_default_libdir "${CMAKE_INSTALL_LIBDIR}")
  set(_ydb_odbc_default_datadir "${CMAKE_INSTALL_DATAROOTDIR}/ydb-odbc")
endif()

set(YDB_ODBC_INSTALL_LIBDIR "${_ydb_odbc_default_libdir}" CACHE STRING
  "Directory where the YDB ODBC shared library is installed")
set(YDB_ODBC_INSTALL_DATADIR "${_ydb_odbc_default_datadir}" CACHE STRING
  "Directory where the YDB ODBC driver registration template is installed")
unset(_ydb_odbc_default_libdir)
unset(_ydb_odbc_default_datadir)

if (IS_ABSOLUTE "${YDB_ODBC_INSTALL_LIBDIR}")
  set(YDB_ODBC_DRIVER_INSTALL_DIR "${YDB_ODBC_INSTALL_LIBDIR}")
else()
  set(YDB_ODBC_DRIVER_INSTALL_DIR
    "${CMAKE_INSTALL_PREFIX}/${YDB_ODBC_INSTALL_LIBDIR}")
endif()

if (IS_ABSOLUTE "${YDB_ODBC_INSTALL_DATADIR}")
  set(YDB_ODBC_DRIVER_TEMPLATE_DIR "${YDB_ODBC_INSTALL_DATADIR}")
else()
  set(YDB_ODBC_DRIVER_TEMPLATE_DIR
    "${CMAKE_INSTALL_PREFIX}/${YDB_ODBC_INSTALL_DATADIR}")
endif()

file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/odbcinst.ini"
  CONTENT "[YDB]
Description=YDB ODBC Driver
Driver=$<TARGET_FILE:ydb-odbc>
Setup=$<TARGET_FILE:ydb-odbc>
"
)

set(YDB_ODBC_DRIVER_PATH
  "${YDB_ODBC_DRIVER_INSTALL_DIR}/libydb-odbc${CMAKE_SHARED_LIBRARY_SUFFIX}")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/odbcinst.ini.in"
  "${CMAKE_CURRENT_BINARY_DIR}/ydb-odbc-odbcinst.ini"
  @ONLY
)

set(YDB_ODBC_DRIVER_TEMPLATE_PATH
  "${YDB_ODBC_DRIVER_TEMPLATE_DIR}/odbcinst.ini")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/debian")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/packaging/postinst.in"
  "${CMAKE_CURRENT_BINARY_DIR}/debian/postinst"
  @ONLY
)
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/packaging/prerm.in"
  "${CMAKE_CURRENT_BINARY_DIR}/debian/prerm"
  @ONLY
)
set(YDB_ODBC_DEBIAN_CONTROL_EXTRA
  "${CMAKE_CURRENT_BINARY_DIR}/debian/postinst;${CMAKE_CURRENT_BINARY_DIR}/debian/prerm"
  CACHE INTERNAL "Debian control scripts for the ydb-odbc package")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ydb-odbc-odbcinst.ini"
  DESTINATION "${YDB_ODBC_INSTALL_DATADIR}"
  RENAME odbcinst.ini
  COMPONENT ydb-odbc
)

install(TARGETS ydb-odbc
  LIBRARY DESTINATION "${YDB_ODBC_INSTALL_LIBDIR}"
  COMPONENT ydb-odbc
)

if (YDB_SDK_EXAMPLES)
  add_subdirectory(examples)
endif()

if (YDB_SDK_TESTS)
  add_subdirectory(tests)
endif()
