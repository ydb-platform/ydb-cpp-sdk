enable_testing()

include(FetchContent)

#todo можно перенести на CPM
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        1.15.2
  FIND_PACKAGE_ARGS NAMES GTest
)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# Set opentelemetry-cpp options as CACHE variables so they're visible to FetchContent
set(WITH_OTLP_HTTP ON CACHE BOOL "" FORCE)
set(WITH_STL CXX20 CACHE STRING "" FORCE)
set(WITH_EXAMPLES OFF CACHE BOOL "" FORCE)
set(WITH_FUNC_TESTS OFF CACHE BOOL "" FORCE)
set(WITH_ABI_VERSION_1 OFF CACHE BOOL "" FORCE)
set(WITH_ABI_VERSION_2 ON CACHE BOOL "" FORCE)
set(WITH_BENCHMARK OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  opentelemetry-cpp
  GIT_REPOSITORY https://github.com/open-telemetry/opentelemetry-cpp.git
  GIT_TAG        v1.24.0
)

FetchContent_MakeAvailable(opentelemetry-cpp)

function(add_ydb_test)
  set(opts GTEST)
  set(oneval_args NAME WORKING_DIRECTORY OUTPUT_DIRECTORY)
  set(multival_args INCLUDE_DIRS SOURCES LINK_LIBRARIES LABELS TEST_ARG ENV)
  cmake_parse_arguments(YDB_TEST
    "${opts}"
    "${oneval_args}"
    "${multival_args}"
    ${ARGN}
  )

  if (YDB_TEST_WORKING_DIRECTORY AND NOT EXISTS "${YDB_TEST_WORKING_DIRECTORY}")
    file(MAKE_DIRECTORY "${YDB_TEST_WORKING_DIRECTORY}")
  endif()

  if (YDB_TEST_OUTPUT_DIRECTORY AND NOT EXISTS "${YDB_TEST_OUTPUT_DIRECTORY}")
    file(MAKE_DIRECTORY "${YDB_TEST_OUTPUT_DIRECTORY}")
  endif()

  add_executable(${YDB_TEST_NAME})
  target_include_directories(${YDB_TEST_NAME} PRIVATE ${YDB_TEST_INCLUDE_DIRS})
  target_link_libraries(${YDB_TEST_NAME} PRIVATE ${YDB_TEST_LINK_LIBRARIES})
  target_sources(${YDB_TEST_NAME} PRIVATE ${YDB_TEST_SOURCES})

  if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
    target_link_libraries(${YDB_TEST_NAME} PRIVATE
      cpuid_check
    )
  endif()

  if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(${YDB_TEST_NAME} PRIVATE
      -ldl
      -lrt
      -Wl,--no-as-needed
      -lpthread
    )
  elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_options(${YDB_TEST_NAME} PRIVATE
      -Wl,-platform_version,macos,11.0,11.0
      -framework
      CoreFoundation
    )
  endif()

  if (YDB_TEST_GTEST)
    set(env_vars "")
    foreach(env_var ${YDB_TEST_ENV})
      list(APPEND env_vars "ENVIRONMENT")
      list(APPEND env_vars ${env_var})
    endforeach()

    gtest_discover_tests(${YDB_TEST_NAME}
      EXTRA_ARGS ${YDB_TEST_TEST_ARG}
      WORKING_DIRECTORY ${YDB_TEST_WORKING_DIRECTORY}
      PROPERTIES
        LABELS ${YDB_TEST_LABELS}
        ENVIRONMENT "YDB_TEST_ROOT=sdk_tests"
        ${env_vars}
    )

    target_link_libraries(${YDB_TEST_NAME} PRIVATE
      GTest::gtest_main
      GTest::gmock_main
    )
  else()
    add_test(NAME ${YDB_TEST_NAME}
      WORKING_DIRECTORY ${YDB_TEST_WORKING_DIRECTORY}
      COMMAND ${YDB_TEST_NAME}
        --print-before-suite
        --print-before-test
        --fork-tests
        --print-times
        --show-fails
        ${YDB_TEST_TEST_ARG}
    )

    target_link_libraries(${YDB_TEST_NAME} PRIVATE
      cpp-testing-unittest_main
    )

    set_tests_properties(${YDB_TEST_NAME} PROPERTIES LABELS ${YDB_TEST_LABELS})
    set_tests_properties(${YDB_TEST_NAME} PROPERTIES ENVIRONMENT "YDB_TEST_ROOT=sdk_tests")
    if (YDB_TEST_ENV)
      set_tests_properties(${YDB_TEST_NAME} PROPERTIES ENVIRONMENT ${YDB_TEST_ENV})
    endif()
  endif()

  vcs_info(${YDB_TEST_NAME})
endfunction()
