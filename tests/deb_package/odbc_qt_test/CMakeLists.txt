cmake_minimum_required(VERSION 3.16)
project(ydb_odbc_qt_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)

add_executable(ydb_odbc_qt_test main.cpp)
target_link_libraries(ydb_odbc_qt_test PRIVATE Qt6::Core Qt6::Sql)
