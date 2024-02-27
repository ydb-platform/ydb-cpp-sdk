add_library(cpp-client-iam_private)

target_link_libraries(cpp-client-iam_private PUBLIC
  contrib-libs-cxxsupp
  yutil
  client-yc_private-iam
  client-iam-common
)

target_sources(cpp-client-iam_private PRIVATE
  ${CMAKE_SOURCE_DIR}/client/iam_private/iam.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-client-iam_private PUBLIC
    contrib-libs-linux-headers
  )
endif()
