add_library(public-lib-operation_id)
target_link_libraries(public-lib-operation_id PUBLIC
    contrib-libs-linux-headers
    contrib-libs-cxxsupp
    yutil
    contrib-libs-protobuf
    library-cpp-cgiparam
    library-cpp-uri
    lib-operation_id-protos
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
    target_link_libraries(public-lib-operation_id PUBLIC
        contrib-libs-linux-headers
    )
endif()

target_sources(public-lib-operation_id PRIVATE
    ${CMAKE_SOURCE_DIR}/ydb/public/lib/operation_id/operation_id.cpp
)