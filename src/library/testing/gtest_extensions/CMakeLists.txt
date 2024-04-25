add_library(cpp-testing-gtest_extensions)
<<<<<<< HEAD
<<<<<<< HEAD

=======
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======

>>>>>>> origin/main
target_link_libraries(cpp-testing-gtest_extensions PUBLIC
  yutil
  GTest::gmock
  GTest::gtest
)
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> origin/main

target_sources(cpp-testing-gtest_extensions PRIVATE
  assertions.cpp
  gtest_extensions.cpp
  matchers.cpp
  pretty_printers.cpp
  probe.cpp
)
<<<<<<< HEAD
=======
target_sources(cpp-testing-gtest_extensions PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/gtest_extensions/assertions.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/gtest_extensions/gtest_extensions.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/gtest_extensions/matchers.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/gtest_extensions/pretty_printers.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/gtest_extensions/probe.cpp
)

# Unit tests
add_subdirectory(ut)
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
