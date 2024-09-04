#[=[ TODO: fix bug with std::signbit
add_ydb_test(NAME int128-int128_ut
  SOURCES
   int128_ut.cpp
  LINK_LIBRARIES
    int128
    cpp-testing-unittest_main
  LABELS
    unit
)
#]=]

add_ydb_test(NAME int128-int128_typetraits_ut
  SOURCES
    int128_typetraits_ut.cpp
  LINK_LIBRARIES
    int128
    cpp-testing-unittest_main
  LABELS
    unit
)

add_ydb_test(NAME int128-i128_ut
  SOURCES
    i128_ut.cpp
  LINK_LIBRARIES
    int128
    cpp-testing-unittest_main
  LABELS
    unit
)

add_ydb_test(NAME int128-i128_and_intrinsic_identity_ut
  SOURCES
    i128_and_intrinsic_identity_ut.cpp
    int128_ut_helpers.cpp
  LINK_LIBRARIES
    int128
    cpp-testing-unittest_main
  LABELS
    unit
)

add_ydb_test(NAME int128-int128_via_intrinsic_ut
  SOURCES
    int128_via_intrinsic_ut.cpp
  LINK_LIBRARIES
    int128
    cpp-testing-unittest_main
  LABELS
    unit
)

add_ydb_test(NAME int128-int128_old_ut
  SOURCES
    int128_old_ut.cpp
    int128_ut_helpers.cpp
  LINK_LIBRARIES
    int128
    cpp-testing-unittest_main
  LABELS
    unit
)

add_ydb_test(NAME int128-i128_division_ut
  SOURCES
    i128_division_ut.cpp
  LINK_LIBRARIES
    int128
    cpp-testing-unittest_main
  LABELS
    unit
)

#[=[ TODO: fix bug with std::signbit
add_ydb_test(NAME int128-i128_type_traits_ut
  SOURCES
    i128_type_traits_ut.cpp
  LINK_LIBRARIES
    int128
    cpp-testing-unittest_main
  LABELS
    unit
)
#]=]

add_ydb_test(NAME int128-i128_comparison_ut
  SOURCES
    i128_comparison_ut.cpp
  LINK_LIBRARIES
    int128
    cpp-testing-unittest_main
  LABELS
    unit
)

add_ydb_test(NAME int128-ui128_division_ut
  SOURCES
    ui128_division_ut.cpp
  LINK_LIBRARIES
    int128
    cpp-testing-unittest_main
  LABELS
    unit
)
