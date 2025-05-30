add_executable(odbc_scheme
    main.cpp
)

target_link_libraries(odbc_scheme
    PRIVATE
        ODBC::ODBC
)
target_compile_definitions(odbc_scheme 
    PRIVATE 
        ODBC_DRIVER_PATH="$<TARGET_FILE:ydb-odbc>"
)

add_dependencies(odbc_scheme ydb-odbc)
