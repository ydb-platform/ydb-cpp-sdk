find_package(Python3 REQUIRED)

set(KOSHER_SVN_VERSION "")
set(SANDBOX_TASK_ID 0)
set(BUILD_TYPE RELEASE)

add_library(build_info)
target_include_directories(build_info
  PRIVATE
    ${YDB_SDK_BINARY_DIR}/library/cpp/build_info
)

target_link_libraries(build_info
  PRIVATE
    yutil
    string_utils-base64
)

target_sources(build_info 
  PRIVATE
    ${YDB_SDK_BINARY_DIR}/library/cpp/build_info/sandbox.cpp
    ${YDB_SDK_BINARY_DIR}/library/cpp/build_info/build_info.cpp
    ${YDB_SDK_BINARY_DIR}/library/cpp/build_info/buildinfo_data.h
    build_info_static.cpp
)

add_custom_command(
  OUTPUT
    ${YDB_SDK_BINARY_DIR}/library/cpp/build_info/buildinfo_data.h
  DEPENDS
    ${YDB_SDK_SOURCE_DIR}/scripts/build_info_gen.py
  COMMAND
    Python3::Interpreter
    ${YDB_SDK_SOURCE_DIR}/scripts/build_info_gen.py
    ${YDB_SDK_BINARY_DIR}/library/cpp/build_info/buildinfo_data.h
    \"${CMAKE_CXX_COMPILER}\"
    \"${CMAKE_CXX_FLAGS}\"
)

configure_file(
  ${YDB_SDK_SOURCE_DIR}/library/cpp/build_info/sandbox.cpp.in
  ${YDB_SDK_BINARY_DIR}/library/cpp/build_info/sandbox.cpp
)
configure_file(
  ${YDB_SDK_SOURCE_DIR}/library/cpp/build_info/build_info.cpp.in
  ${YDB_SDK_BINARY_DIR}/library/cpp/build_info/build_info.cpp
)

_ydb_sdk_install_targets(TARGETS build_info)
