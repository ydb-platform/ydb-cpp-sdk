get_built_tool_path(
    TOOL_protoc_bin
    TOOL_protoc_dependency
    contrib/tools/protoc/bin
    protoc
)

get_built_tool_path(
    TOOL_cpp_styleguide_bin
    TOOL_cpp_styleguide_dependency
    contrib/tools/protoc/plugins/cpp_styleguide
    cpp_styleguide
)

add_library(api-protos-annotations)
target_link_libraries(api-protos-annotations PUBLIC
    contrib-libs-cxxsupp
    yutil
    contrib-libs-protobuf
)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
    target_link_libraries(api-protos-annotations PUBLIC
        contrib-libs-linux-headers
    )
endif()

target_proto_messages(api-protos-annotations PRIVATE
    ${CMAKE_SOURCE_DIR}/ydb/public/api/protos/annotations/sensitive.proto
    ${CMAKE_SOURCE_DIR}/ydb/public/api/protos/annotations/validation.proto
)
target_proto_addincls(api-protos-annotations
    ./
    ${CMAKE_SOURCE_DIR}/
    ${CMAKE_BINARY_DIR}/
    ${CMAKE_SOURCE_DIR}/contrib/libs/protobuf/src
)
target_proto_outs(api-protos-annotations
    --cpp_out=${CMAKE_BINARY_DIR}/
    --cpp_styleguide_out=${CMAKE_BINARY_DIR}/
)