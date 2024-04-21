find_package(Python3 REQUIRED)

set(KOSHER_SVN_VERSION "")
set(SANDBOX_TASK_ID 0)
set(BUILD_TYPE RELEASE)

add_library(library-cpp-build_info)
target_include_directories(library-cpp-build_info PUBLIC
  ${YDB_SDK_BINARY_DIR}/src/library/build_info
)
target_link_libraries(library-cpp-build_info PUBLIC
  yutil
  cpp-string_utils-base64
)
target_sources(library-cpp-build_info PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/build_info/build_info_static.cpp
  ${YDB_SDK_BINARY_DIR}/src/library/build_info/buildinfo_data.h
  ${YDB_SDK_BINARY_DIR}/src/library/build_info/sandbox.cpp
  ${YDB_SDK_BINARY_DIR}/src/library/build_info/build_info.cpp
)
add_custom_command(
  OUTPUT
  ${YDB_SDK_BINARY_DIR}/src/library/build_info/buildinfo_data.h
  DEPENDS
  ${YDB_SDK_SOURCE_DIR}/scripts/build_info_gen.py
  COMMAND
  Python3::Interpreter
  ${YDB_SDK_SOURCE_DIR}/scripts/build_info_gen.py
  ${YDB_SDK_BINARY_DIR}/src/library/build_info/buildinfo_data.h
  \"${CMAKE_CXX_COMPILER}\"
  \"${CMAKE_CXX_FLAGS}\"
)
configure_file(
  ${YDB_SDK_SOURCE_DIR}/src/library/build_info/sandbox.cpp.in
  ${YDB_SDK_BINARY_DIR}/src/library/build_info/sandbox.cpp
)
configure_file(
  ${YDB_SDK_SOURCE_DIR}/src/library/build_info/build_info.cpp.in
  ${YDB_SDK_BINARY_DIR}/src/library/build_info/build_info.cpp
)
