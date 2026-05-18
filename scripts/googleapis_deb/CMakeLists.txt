cmake_minimum_required(VERSION 3.10)
project(yandex-googleapis-api-common-protos)

set(CMAKE_CXX_STANDARD 20)

find_package(Protobuf REQUIRED)

set(PROTOS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/api-common-protos")

file(GLOB_RECURSE PROTO_FILES "${PROTOS_DIR}/google/*.proto")

# We need to generate the cpp files
set(PROTO_SRCS)
set(PROTO_HDRS)

foreach(proto_file ${PROTO_FILES})
    file(RELATIVE_PATH rel_path "${PROTOS_DIR}" "${proto_file}")
    string(REPLACE ".proto" ".pb.cc" cpp_file "${CMAKE_CURRENT_BINARY_DIR}/${rel_path}")
    string(REPLACE ".proto" ".pb.h" h_file "${CMAKE_CURRENT_BINARY_DIR}/${rel_path}")
    
    get_filename_component(out_dir "${cpp_file}" DIRECTORY)
    file(MAKE_DIRECTORY "${out_dir}")

    add_custom_command(
        OUTPUT "${cpp_file}" "${h_file}"
        COMMAND protobuf::protoc
        ARGS --cpp_out="${CMAKE_CURRENT_BINARY_DIR}" -I"${PROTOS_DIR}" "${proto_file}"
        DEPENDS "${proto_file}"
        COMMENT "Running C++ protocol buffer compiler on ${proto_file}"
        VERBATIM
    )

    list(APPEND PROTO_SRCS "${cpp_file}")
    list(APPEND PROTO_HDRS "${h_file}")
endforeach()

add_library(api-common-protos STATIC ${PROTO_SRCS} ${PROTO_HDRS})
add_library(yandex-googleapis-api-common-protos::api-common-protos ALIAS api-common-protos)

target_include_directories(api-common-protos PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(api-common-protos PUBLIC protobuf::libprotobuf)

include(GNUInstallDirs)

install(TARGETS api-common-protos
    EXPORT yandex-googleapis-api-common-protos-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/google"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.pb.h"
)

install(EXPORT yandex-googleapis-api-common-protos-targets
    FILE yandex-googleapis-api-common-protosTargets.cmake
    NAMESPACE yandex-googleapis-api-common-protos::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/yandex-googleapis-api-common-protos
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/yandex-googleapis-api-common-protosConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/yandex-googleapis-api-common-protosConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/yandex-googleapis-api-common-protos
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/yandex-googleapis-api-common-protosConfig.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/yandex-googleapis-api-common-protos
)

set(CPACK_PACKAGE_NAME "yandex-googleapis-api-common-protos")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "YDB Team <whoami@where>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libprotobuf-dev")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/share/yandex")

include(CPack)
