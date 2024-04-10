add_library(cpp-testing-gmock_in_unittest)
target_link_libraries(cpp-testing-gmock_in_unittest PUBLIC
  yutil
  GTest::gmock
  GTest::gtest
  cpp-testing-gtest_extensions
  cpp-testing-unittest
)

target_sources(cpp-testing-gmock_in_unittest PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/library/testing/gmock_in_unittest/CMakeLists.txt
  events.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/testing/gmock_in_unittest/events.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/testing/gmock_in_unittest/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/gmock_in_unittest/events.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/testing/gmock_in_unittest/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)

add_global_library_for(cpp-testing-gmock_in_unittest.global cpp-testing-gmock_in_unittest)
target_link_libraries(cpp-testing-gmock_in_unittest.global PUBLIC
  yutil
  GTest::gmock
  GTest::gtest
  cpp-testing-gtest_extensions
  cpp-testing-unittest
)
<<<<<<< HEAD:src/library/testing/gmock_in_unittest/CMakeLists.txt
=======
target_sources(cpp-testing-gmock_in_unittest.global PRIVATE
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/gmock_in_unittest/registration.cpp
)
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/testing/gmock_in_unittest/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/testing/gmock_in_unittest/registration.cpp
)
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/testing/gmock_in_unittest/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))

target_sources(cpp-testing-gmock_in_unittest.global PRIVATE
  registration.cpp
)
