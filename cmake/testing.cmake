enable_testing()

include(GoogleTest)

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
  _ydb_sdk_apply_coverage(${YDB_TEST_NAME})
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
      -framework
      CoreFoundation
    )
  endif()

  if (YDB_TEST_GTEST)
    set(env_vars "")
    foreach(env_var IN LISTS YDB_TEST_ENV)
      list(APPEND env_vars "ENVIRONMENT")
      list(APPEND env_vars "${env_var}")
    endforeach()
    gtest_discover_tests(${YDB_TEST_NAME}
      EXTRA_ARGS ${YDB_TEST_TEST_ARG}
      WORKING_DIRECTORY ${YDB_TEST_WORKING_DIRECTORY}
      PROPERTIES
        ENVIRONMENT "YDB_TEST_ROOT=sdk_tests"
        ${env_vars}
    )

    # Discovered tests only exist when CTest loads this directory. Assign
    # labels from a second include so a semicolon-separated label list stays a
    # single property value rather than becoming extra property/value pairs.
    if (YDB_TEST_LABELS)
      set(test_labels_file
        "${CMAKE_CURRENT_BINARY_DIR}/${YDB_TEST_NAME}_labels.cmake")
      string(CONCAT test_labels_content
        "if(DEFINED ${YDB_TEST_NAME}_TESTS)\n"
        "  set_tests_properties(\${${YDB_TEST_NAME}_TESTS} PROPERTIES LABELS \"${YDB_TEST_LABELS}\")\n"
        "endif()\n")
      file(GENERATE OUTPUT "${test_labels_file}" CONTENT "${test_labels_content}")
      set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES "${test_labels_file}")
    endif()

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

    set_tests_properties(${YDB_TEST_NAME} PROPERTIES LABELS "${YDB_TEST_LABELS}")
    set_tests_properties(${YDB_TEST_NAME} PROPERTIES ENVIRONMENT "YDB_TEST_ROOT=sdk_tests")
    if (YDB_TEST_ENV)
      set_tests_properties(${YDB_TEST_NAME} PROPERTIES ENVIRONMENT ${YDB_TEST_ENV})
    endif()
  endif()

  vcs_info(${YDB_TEST_NAME})
endfunction()

if (YDB_SDK_ODBC)
  function(add_odbc_test)
    set(opts "")
    set(oneval_args NAME WORKING_DIRECTORY OUTPUT_DIRECTORY)
    set(multival_args SOURCES LINK_LIBRARIES LABELS)
    cmake_parse_arguments(ODBC_TEST
      "${opts}"
      "${oneval_args}"
      "${multival_args}"
      ${ARGN}
    )

    add_ydb_test(GTEST
      NAME ${ODBC_TEST_NAME}
      SOURCES ${ODBC_TEST_SOURCES}
      LINK_LIBRARIES
        ${ODBC_TEST_LINK_LIBRARIES}
        ODBC::ODBC
      LABELS
        integration
        ${ODBC_TEST_LABELS}
    )

    target_compile_definitions(${ODBC_TEST_NAME} 
      PRIVATE 
        ODBC_DRIVER_PATH="$<TARGET_FILE:ydb-odbc>"
        ODBC_TEST_ODBCINI="${CMAKE_BINARY_DIR}/odbc/odbc.ini"
        ODBC_TEST_ODBCSYSINI="${CMAKE_BINARY_DIR}/odbc"
    )

    add_dependencies(${ODBC_TEST_NAME} ydb-odbc)
  endfunction()
endif()
