# Standalone platform adjustments belong here, outside imported directories.
if(NOT WIN32)
  return()
endif()

if(MSVC)
  foreach(_target IN ITEMS blockcodecs-codecs-lz4.global
      coroutine-engine-coroutine_ut digest-lower_case-ut digest-lower_case
      api-protos client-ydb_discovery client-ydb_federated_topic-impl
      client-ydb_table-impl client-ydb_topic-impl yutil util-charset)
    if(TARGET ${_target})
      get_target_property(_options ${_target} COMPILE_OPTIONS)
      if(_options)
        list(REMOVE_ITEM _options -Wno-deprecated -Wnarrowing)
        set_property(TARGET ${_target} PROPERTY COMPILE_OPTIONS "${_options}")
      endif()
    endif()
  endforeach()
endif()

if(TARGET yutil)
  target_link_libraries(yutil PUBLIC ws2_32 iphlpapi psapi advapi32)
  # Backslashes in these stringized macros would become C++ escape sequences.
  foreach(_property IN ITEMS COMPILE_DEFINITIONS INTERFACE_COMPILE_DEFINITIONS)
    get_target_property(_definitions yutil ${_property})
    list(FILTER _definitions EXCLUDE REGEX "^ARCADIA_(BUILD_)?ROOT=")
    set_property(TARGET yutil PROPERTY ${_property} "${_definitions}")
  endforeach()
  file(TO_CMAKE_PATH "${ARCADIA_ROOT}" _source_root)
  file(TO_CMAKE_PATH "${ARCADIA_BUILD_ROOT}" _build_root)
  target_compile_definitions(yutil PUBLIC
    ARCADIA_ROOT=${_source_root} ARCADIA_BUILD_ROOT=${_build_root})
endif()
