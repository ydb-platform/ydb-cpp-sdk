add_library(api-protos-out)
target_link_libraries(api-protos-out PUBLIC
    contrib-libs-linux-headers
    contrib-libs-cxxsupp
    yutil
    api-protos
)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
    target_link_libraries(api-protos-out PUBLIC
        contrib-libs-linux-headers
    )
endif()
target_sources(api-protos-out PRIVATE
    ${CMAKE_SOURCE_DIR}/ydb/public/api/protos/out/out.cpp
)
