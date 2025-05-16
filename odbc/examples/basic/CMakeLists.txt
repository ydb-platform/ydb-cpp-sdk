add_executable(odbc_basic
    main.cpp
)

target_link_libraries(odbc_basic
    PRIVATE
        ODBC::ODBC
)
target_compile_definitions(odbc_basic 
    PRIVATE 
        ODBC_DRIVER_PATH="$<TARGET_FILE:ydb-odbc>"
)

add_dependencies(odbc_basic ydb-odbc)
