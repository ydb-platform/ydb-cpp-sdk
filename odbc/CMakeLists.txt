cmake_minimum_required(VERSION 3.14)
project(ydb-odbc VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Находим зависимости
find_package(ODBC REQUIRED)

# Добавляем исходники
add_library(ydb-odbc SHARED
  src/driver.c
  src/connection.c
  src/statement.c
  src/descriptor.c
  src/client/driver.cpp
  src/client/query.cpp
)

# Добавляем заголовочные файлы
target_include_directories(ydb-odbc
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ODBC_INCLUDE_DIRS}
    /usr/include
    /usr/local/include
)

# Линкуем с YDB SDK и ODBC
target_link_libraries(ydb-odbc
  PUBLIC
    ydb-cpp-sdk::Query
    ydb-cpp-sdk::Table
    ydb-cpp-sdk::Driver
    ODBC::ODBC
)

# Устанавливаем драйвер
install(TARGETS ydb-odbc
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Устанавливаем заголовочные файлы
install(DIRECTORY include/
  DESTINATION include/ydb-odbc
)

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
