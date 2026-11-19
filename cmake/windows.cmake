# Standalone platform adjustments belong here, outside imported directories.
if(NOT MINGW)
  return()
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

# Only yutil directly compiles sources in the adapted directories. Joined
# sources already find their compatibility copies through the include path.
get_target_property(_sources yutil SOURCES)
foreach(_source IN LISTS _sources)
  string(REPLACE "${YDB_SDK_SOURCE_DIR}/" "" _relative "${_source}")
  if(_relative MATCHES "^util/(system|datetime|folder)/")
    list(REMOVE_ITEM _sources "${_source}")
    list(APPEND _sources "${YDB_SDK_MINGW_COMPAT_DIR}/${_relative}")
  endif()
endforeach()
set_property(TARGET yutil PROPERTY SOURCES "${_sources}")
if(TARGET yt-memory)
  # GCC 16 requires the declaration before shared_range templates use New<T>.
  target_compile_options(yt-memory PRIVATE
    -include "${YDB_SDK_SOURCE_DIR}/library/cpp/yt/memory/new.h")
endif()
