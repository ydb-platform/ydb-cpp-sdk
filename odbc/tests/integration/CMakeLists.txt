add_odbc_test(NAME odbc-basic_it
  SOURCES
    basic_it.cpp
)

add_odbc_test(NAME odbc-environment_api_it
  SOURCES
    environment_api_it.cpp
)

# Core API Tests
add_odbc_test(NAME odbc-connection_api_it
  SOURCES
    connection_api_it.cpp
)

add_odbc_test(NAME odbc-statement_api_it
  SOURCES
    statement_api_it.cpp
)

add_odbc_test(NAME odbc-transaction_api_it
  SOURCES
    transaction_api_it.cpp
)

add_odbc_test(NAME odbc-error_handling_it
  SOURCES
    error_handling_it.cpp
)

add_odbc_test(NAME odbc-metadata_api_it
  SOURCES
    metadata_api_it.cpp
)
