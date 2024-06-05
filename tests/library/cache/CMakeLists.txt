add_ydb_test(NAME cache-cache_ut
        SOURCES
        cache_ut.cpp
        LINK_LIBRARIES
        yutil
        cpp-testing-unittest_main
        library-cpp-cache
)