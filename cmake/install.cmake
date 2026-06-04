function(_ydb_sdk_install_targets)
  if (NOT YDB_SDK_INSTALL)
    return()
  endif()
  set(oneValueArgs COMPONENT)
  set(multiValueArgs TARGETS)
  cmake_parse_arguments(
    ARG "${option}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN}
  )
  if (NOT ARG_TARGETS)
    message(FATAL_ERROR "No TARGETS given for install")
    return()
  endif()
  if (NOT ARG_COMPONENT)
    set(ARG_COMPONENT "libydb-cpp")
  endif()
  foreach (ITEM_TARGET IN LISTS ARG_TARGETS)
    if(NOT TARGET ${ITEM_TARGET})
      message(FATAL_ERROR "${ITEM_TARGET} is not target. You should use only targets")
      return()
    endif()
  endforeach()
  if (ARG_COMPONENT STREQUAL "libydb-cpp")
    set(_ydb_sdk_filtered_targets "")
    foreach(_ydb_sdk_t IN LISTS ARG_TARGETS)
      if (_ydb_sdk_t STREQUAL "libydb-cpp")
        list(APPEND _ydb_sdk_filtered_targets "${_ydb_sdk_t}")
      endif()
    endforeach()
    set(ARG_TARGETS "${_ydb_sdk_filtered_targets}")
    if (NOT ARG_TARGETS)
      return()
    endif()
  endif()
  install(TARGETS ${ARG_TARGETS}
    EXPORT ydb-cpp-sdk-targets
    CONFIGURATIONS RELEASE
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT ${ARG_COMPONENT}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT ${ARG_COMPONENT}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${ARG_COMPONENT}
    INCLUDES DESTINATION
      ${CMAKE_INSTALL_INCLUDEDIR}
      ${CMAKE_INSTALL_INCLUDEDIR}/__ydb_sdk_special_headers
  )
endfunction()

function(_ydb_sdk_directory_install)
  if (NOT ${YDB_SDK_INSTALL})
    return()
  endif()
  set(oneValueArgs DESTINATION PATTERN COMPONENT EXCLUDE)
  set(multiValueArgs FILES DIRECTORY)
  cmake_parse_arguments(
    ARG "${option}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN}
  )
  if (NOT ARG_DESTINATION)
    message(FATAL_ERROR "No DESTINATION for install")
  endif()
  if (NOT ARG_FILES AND NOT ARG_DIRECTORY)
    message(FATAL_ERROR "No FILES or DIRECTORY provided to install")
  endif()
  if (ARG_FILES AND ARG_DIRECTORY)
    message(FATAL_ERROR "FILES and DIRECTORY are mutually exclusive install arguments")
  endif()
  if (NOT ARG_COMPONENT)
    set(ARG_COMPONENT "libydb-cpp")
  endif()
  if (ARG_FILES)
    install(FILES ${ARG_FILES} DESTINATION ${ARG_DESTINATION} COMPONENT ${ARG_COMPONENT})
  else()
    if (DEFINED ARG_PATTERN)
      install(DIRECTORY ${ARG_DIRECTORY} DESTINATION ${ARG_DESTINATION} USE_SOURCE_PERMISSIONS COMPONENT ${ARG_COMPONENT} FILES_MATCHING PATTERN ${ARG_PATTERN})
    elseif (DEFINED ARG_EXCLUDE)
      install(DIRECTORY ${ARG_DIRECTORY} DESTINATION ${ARG_DESTINATION} USE_SOURCE_PERMISSIONS COMPONENT ${ARG_COMPONENT} PATTERN ${ARG_EXCLUDE} EXCLUDE)
    else()
      install(DIRECTORY ${ARG_DIRECTORY} DESTINATION ${ARG_DESTINATION} USE_SOURCE_PERMISSIONS COMPONENT ${ARG_COMPONENT})
    endif()
  endif()
endfunction()

function(_ydb_sdk_install_headers ArgIncludeDir)
  if (NOT ${YDB_SDK_INSTALL})
    return()
  endif()
  set(oneValueArgs COMPONENT)
  set(multiValueArgs DIRECTORY)
  cmake_parse_arguments(
    ARG "${option}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN}
  )
  if (NOT ARG_COMPONENT)
    set(ARG_COMPONENT "libydb-cpp")
  endif()

  if (ARG_DIRECTORY)
    _ydb_sdk_directory_install(DIRECTORY ${ARG_DIRECTORY}
      DESTINATION ${ArgIncludeDir}
      COMPONENT ${ARG_COMPONENT}
    )
  else()
    _ydb_sdk_directory_install(DIRECTORY ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk
      DESTINATION ${ArgIncludeDir}
      COMPONENT ${ARG_COMPONENT}
      EXCLUDE "open_telemetry"
    )

    file(STRINGS ${YDB_SDK_SOURCE_DIR}/cmake/public_headers.txt PublicHeaders)
    file(STRINGS ${YDB_SDK_SOURCE_DIR}/cmake/protos_public_headers.txt ProtosPublicHeaders)
    if (NOT MSVC)
      list(REMOVE_ITEM PublicHeaders library/cpp/deprecated/atomic/atomic_win.h)
    endif()
    foreach(HeaderPath ${PublicHeaders})
      get_filename_component(RelInstallPath ${HeaderPath} DIRECTORY)
      _ydb_sdk_directory_install(FILES
          ${YDB_SDK_SOURCE_DIR}/${HeaderPath}
        DESTINATION ${ArgIncludeDir}/__ydb_sdk_special_headers/${RelInstallPath}
        COMPONENT ${ARG_COMPONENT}
      )
    endforeach()
    foreach(HeaderPath ${ProtosPublicHeaders})
      get_filename_component(RelInstallPath ${HeaderPath} DIRECTORY)
      _ydb_sdk_directory_install(FILES
          ${YDB_SDK_BINARY_DIR}/${HeaderPath}
        DESTINATION ${ArgIncludeDir}/__ydb_sdk_special_headers/${RelInstallPath}
        COMPONENT ${ARG_COMPONENT}
      )
    endforeach()
  endif()
endfunction()
