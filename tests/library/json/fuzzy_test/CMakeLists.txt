add_ydb_test(NAME json-fuzzy_test-main
SOURCES
    main.cpp    
LINK_LIBRARIES
    yutil
    library-cpp-json
    cpp-testing-unittest_main
)