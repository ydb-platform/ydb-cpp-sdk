add_subdirectory(macro)
add_subdirectory(ref)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  add_subdirectory(avx2)
  add_subdirectory(sse2)
  add_subdirectory(sse41)
  add_subdirectory(ssse3)
endif ()
