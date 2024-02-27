add_subdirectory(common)

add_library(cpp-client-iam)

target_link_libraries(cpp-client-iam PUBLIC
  contrib-libs-cxxsupp
  yutil
  client-yc_public-iam
  client-iam-common
)

target_sources(cpp-client-iam PRIVATE
  ${CMAKE_SOURCE_DIR}/client/iam/iam.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-client-iam PUBLIC
    contrib-libs-linux-headers
  )
endif()
