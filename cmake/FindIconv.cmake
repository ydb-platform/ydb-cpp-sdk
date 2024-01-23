# - Find Iconv
#
# Iconv_INCLUDE - Where to find LibIconv public headers
# Iconv_LIBS - List of libraries when using LibIconv.
# Iconv_FOUND - True if LibIconv found.

find_path(Iconv_INCLUDE_DIR
  iconv.h
  HINTS $ENV{Iconv_ROOT}/include)

find_library(Iconv_LIBRARIES
  iconv
  HINTS $ENV{Iconv_ROOT}/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Iconv DEFAULT_MSG Iconv_LIBRARIES Iconv_INCLUDE_DIR)

mark_as_advanced(Iconv_INCLUDE_DIR Iconv_LIBRARIES)

if (Iconv_FOUND AND NOT TARGET Iconv::Iconv)
  add_library(Iconv::Iconv UNKNOWN IMPORTED)
  set_target_properties(Iconv::Iconv PROPERTIES
    IMPORTED_LOCATION ${Iconv_LIBRARIES}
    INTERFACE_INCLUDE_DIRECTORIES ${Iconv_INCLUDE_DIR}
  )
endif()
