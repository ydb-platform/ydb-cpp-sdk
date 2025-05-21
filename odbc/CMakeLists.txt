# Добавляем исходники
add_library(ydb-odbc SHARED
  src/odbc_driver.cpp
  src/connection.cpp
  src/statement.cpp
  src/environment.cpp
)

# Добавляем заголовочные файлы
target_include_directories(ydb-odbc
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ODBC_INCLUDE_DIRS}
)

# Линкуем с YDB SDK и ODBC
target_link_libraries(ydb-odbc
  PRIVATE
    ydb-cpp-sdk::Query
    ydb-cpp-sdk::Table
    ydb-cpp-sdk::Driver
    ODBC::ODBC
)

set_target_properties(ydb-odbc PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

# Устанавливаем драйвер
install(TARGETS ydb-odbc
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Устанавливаем заголовочные файлы
install(DIRECTORY include/
  DESTINATION include/ydb-odbc
)

add_subdirectory(examples)

# Добавляем тесты
# add_subdirectory(tests)

# Правила установки
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
