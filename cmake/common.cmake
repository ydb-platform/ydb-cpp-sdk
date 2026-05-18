# Set of common macros

find_package(Python3 REQUIRED)

# assumes ToolName is always both the binary and the target name
function(get_built_tool_path OutBinPath SrcPath ToolName)
  set(${OutBinPath} "${YDB_SDK_BINARY_DIR}/${SrcPath}/${ToolName}${CMAKE_EXECUTABLE_SUFFIX}" PARENT_SCOPE)
endfunction()

function(target_ragel_lexers TgtName Key Src)
  SET(RAGEL_BIN ragel${CMAKE_EXECUTABLE_SUFFIX})
  get_filename_component(OutPath ${Src} NAME_WLE)
  get_filename_component(SrcDirPath ${Src} DIRECTORY)
  get_filename_component(OutputExt ${OutPath} EXT)
  if (OutputExt STREQUAL "")
    string(APPEND OutPath .rl6.cpp)
  endif()
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${OutPath}
    COMMAND Python3::Interpreter ${YDB_SDK_SOURCE_DIR}/scripts/run_tool.py -- ${RAGEL_BIN} ${RAGEL_FLAGS} ${ARGN} -o ${CMAKE_CURRENT_BINARY_DIR}/${OutPath} ${Src}
    DEPENDS ${YDB_SDK_SOURCE_DIR}/scripts/run_tool.py ${Src}
    WORKING_DIRECTORY ${SrcDirPath}
  )
  target_sources(${TgtName} ${Key} ${CMAKE_CURRENT_BINARY_DIR}/${OutPath})
endfunction()

function(target_yasm_source TgtName Key Src)
  SET(YASM_BIN yasm${CMAKE_EXECUTABLE_SUFFIX})
  get_filename_component(OutPath ${Src} NAME_WLE)
  string(APPEND OutPath .o)
  add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${OutPath}
      COMMAND Python3::Interpreter ${YDB_SDK_SOURCE_DIR}/scripts/run_tool.py -- ${YASM_BIN} ${YASM_FLAGS} ${ARGN} -o ${CMAKE_CURRENT_BINARY_DIR}/${OutPath} ${Src}
    DEPENDS ${YDB_SDK_SOURCE_DIR}/scripts/run_tool.py ${Src}
  )
  target_sources(${TgtName} ${Key} ${CMAKE_CURRENT_BINARY_DIR}/${OutPath})
endfunction()

function(target_joined_source TgtName Out)
  foreach(InSrc ${ARGN})
    file(RELATIVE_PATH IncludePath ${YDB_SDK_SOURCE_DIR} ${InSrc})
    list(APPEND IncludesList ${IncludePath})
  endforeach()
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${Out}
    COMMAND Python3::Interpreter ${YDB_SDK_SOURCE_DIR}/scripts/gen_join_srcs.py ${CMAKE_CURRENT_BINARY_DIR}/${Out} ${IncludesList}
    DEPENDS ${YDB_SDK_SOURCE_DIR}/scripts/gen_join_srcs.py ${ARGN}
  )
  target_sources(${TgtName} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${Out})
endfunction()

function(target_sources_custom TgtName CompileOutSuffix)
  set(opts "")
  set(oneval_args "")
  set(multival_args SRCS CUSTOM_FLAGS)
  cmake_parse_arguments(TARGET_SOURCES_CUSTOM
    "${opts}"
    "${oneval_args}"
    "${multival_args}"
    ${ARGN}
  )

  foreach(Src ${TARGET_SOURCES_CUSTOM_SRCS})
    file(RELATIVE_PATH SrcRealPath ${YDB_SDK_SOURCE_DIR} ${Src})
    get_filename_component(SrcDir ${SrcRealPath} DIRECTORY)
    get_filename_component(SrcName ${SrcRealPath} NAME_WLE)
    get_filename_component(SrcExt ${SrcRealPath} LAST_EXT)
    set(SrcCopy "${YDB_SDK_BINARY_DIR}/${SrcDir}/${SrcName}${CompileOutSuffix}${SrcExt}")
    add_custom_command(
      OUTPUT ${SrcCopy}
      COMMAND ${CMAKE_COMMAND} -E copy ${Src} ${SrcCopy}
      DEPENDS ${Src}
    )
    list(APPEND PreparedSrc ${SrcCopy})
    set_property(
      SOURCE
      ${SrcCopy}
      APPEND PROPERTY COMPILE_OPTIONS
      ${TARGET_SOURCES_CUSTOM_CUSTOM_FLAGS}
      -I${YDB_SDK_SOURCE_DIR}/${SrcDir}
    )
  endforeach()

  target_sources(
    ${TgtName}
    PRIVATE
    ${PreparedSrc}
  )
endfunction()

function(generate_enum_serilization Tgt Input)
  set(opts "")
  set(oneval_args INCLUDE_HEADERS)
  set(multival_args "")
  cmake_parse_arguments(ENUM_SERIALIZATION_ARGS
    "${opts}"
    "${oneval_args}"
    "${multival_args}"
    ${ARGN}
  )

  get_built_tool_path(enum_parser_bin tools/enum_parser/enum_parser enum_parser)

  get_filename_component(BaseName ${Input} NAME)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${BaseName}_serialized.cpp
    COMMAND
      ${enum_parser_bin}
      ${Input}
      --include-path ${ENUM_SERIALIZATION_ARGS_INCLUDE_HEADERS}
      --output ${CMAKE_CURRENT_BINARY_DIR}/${BaseName}_serialized.cpp
    DEPENDS ${Input} ${enum_parser_bin}
  )
  target_sources(${Tgt} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${BaseName}_serialized.cpp)
endfunction()

function(add_global_library_for TgtName MainName)
  add_library(${TgtName} STATIC ${ARGN})
  if(APPLE)
    target_link_options(${MainName} INTERFACE "SHELL:-Wl,-force_load,$<TARGET_FILE:$<INSTALL_INTERFACE:YDB-CPP-SDK::>${TgtName}>")
  else()
    target_link_options(${MainName} INTERFACE "SHELL:-Wl,--whole-archive $<TARGET_FILE:$<INSTALL_INTERFACE:YDB-CPP-SDK::>${TgtName}> -Wl,--no-whole-archive")
  endif()
  add_dependencies(${MainName} ${TgtName})
  target_link_libraries(${MainName} INTERFACE ${TgtName})
endfunction()

function(vcs_info Tgt)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/vcs_info.json
    COMMAND Python3::Interpreter ${YDB_SDK_SOURCE_DIR}/scripts/generate_vcs_info.py ${CMAKE_CURRENT_BINARY_DIR}/vcs_info.json ${YDB_SDK_SOURCE_DIR}
    DEPENDS ${YDB_SDK_SOURCE_DIR}/scripts/generate_vcs_info.py
  )

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/__vcs_version__.c
    COMMAND Python3::Interpreter ${YDB_SDK_SOURCE_DIR}/scripts/vcs_info.py ${CMAKE_CURRENT_BINARY_DIR}/vcs_info.json ${CMAKE_CURRENT_BINARY_DIR}/__vcs_version__.c ${YDB_SDK_SOURCE_DIR}/scripts/c_templates/svn_interface.c
    DEPENDS ${YDB_SDK_SOURCE_DIR}/scripts/vcs_info.py ${YDB_SDK_SOURCE_DIR}/scripts/c_templates/svn_interface.c ${CMAKE_CURRENT_BINARY_DIR}/vcs_info.json
  )
  target_sources(${Tgt} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/__vcs_version__.c)
endfunction()

function(resources Tgt Output)
  set(opts "")
  set(oneval_args "")
  set(multival_args INPUTS KEYS)
  cmake_parse_arguments(RESOURCE_ARGS
    "${opts}"
    "${oneval_args}"
    "${multival_args}"
    ${ARGN}
  )
  list(LENGTH RESOURCE_ARGS_INPUTS InputsCount)
  list(LENGTH RESOURCE_ARGS_KEYS KeysCount)
  if (NOT ${InputsCount} EQUAL ${KeysCount})
    message(FATAL_ERROR "Resources inputs count isn't equal to keys count in " ${Tgt})
  endif()
  math(EXPR ListsMaxIdx "${InputsCount} - 1")
  foreach(Idx RANGE ${ListsMaxIdx})
    list(GET RESOURCE_ARGS_INPUTS ${Idx} Input)
    list(GET RESOURCE_ARGS_KEYS ${Idx} Key)
    list(APPEND ResourcesList ${Input})
    list(APPEND ResourcesList ${Key})
  endforeach()

  get_built_tool_path(rescompiler_bin tools/rescompiler rescompiler)

  add_custom_command(
    OUTPUT ${Output}
    COMMAND ${rescompiler_bin} ${Output} ${ResourcesList}
    DEPENDS ${RESOURCE_ARGS_INPUTS} ${rescompiler_bin}
  )
endfunction()

function(_ydb_sdk_make_client_component CmpName Tgt)
  add_library(YDB-CPP-SDK::${CmpName} ALIAS ${Tgt})

  if (CmpName STREQUAL "Iam" OR CmpName STREQUAL "IamPrivate")
    set(PKG_COMP_NAME "libydb-cpp-iam")
  elseif (CmpName STREQUAL "OpenTelemetryMetrics")
    set(PKG_COMP_NAME "libydb-cpp-otel-metrics")
  elseif (CmpName STREQUAL "OpenTelemetryTrace")
    set(PKG_COMP_NAME "libydb-cpp-otel-tracing")
  else()
    set(PKG_COMP_NAME "libydb-cpp")
  endif()

  set(YDB-CPP-SDK_AVAILABLE_COMPONENTS ${YDB-CPP-SDK_AVAILABLE_COMPONENTS} ${CmpName} CACHE INTERNAL "")

  if (PKG_COMP_NAME STREQUAL "libydb-cpp")
    set(YDB_CPP_CORE_COMPONENT_TARGETS ${YDB_CPP_CORE_COMPONENT_TARGETS} ${Tgt} ${ARGN} CACHE INTERNAL "")
    set(YDB-CPP-SDK_COMPONENT_TARGETS ${YDB-CPP-SDK_COMPONENT_TARGETS} libydb-cpp CACHE INTERNAL "")
  else()
    string(REPLACE "-" "_" PKG_PROP_NAME "${PKG_COMP_NAME}")
    string(TOUPPER "${PKG_PROP_NAME}" PKG_PROP_NAME)
    set(YDB_CPP_${PKG_PROP_NAME}_COMPONENT_TARGETS ${YDB_CPP_${PKG_PROP_NAME}_COMPONENT_TARGETS} ${Tgt} ${ARGN} CACHE INTERNAL "")
    string(REGEX REPLACE "^lib" "" PKG_TARGET_NAME "${PKG_COMP_NAME}")
    set(YDB-CPP-SDK_COMPONENT_TARGETS ${YDB-CPP-SDK_COMPONENT_TARGETS} ${PKG_TARGET_NAME} CACHE INTERNAL "")
  endif()
endfunction()

function(_ydb_sdk_add_library Tgt)
  cmake_parse_arguments(ARG
    "INTERFACE" "" ""
    ${ARGN}
  )

  set(libraryMode "")
  set(includeMode "PUBLIC")
  if (ARG_INTERFACE)
    set(libraryMode "INTERFACE")
    set(includeMode "INTERFACE")
  endif()
  
  add_library(${Tgt} ${libraryMode})
  target_include_directories(${Tgt} ${includeMode}
    $<BUILD_INTERFACE:${YDB_SDK_SOURCE_DIR}>
    $<BUILD_INTERFACE:${YDB_SDK_BINARY_DIR}>
    $<BUILD_INTERFACE:${YDB_SDK_SOURCE_DIR}/include>
  )
  target_compile_definitions(${Tgt} ${includeMode}
    YDB_SDK_OSS
  )
  
endfunction()

function(_ydb_sdk_b64_set_codec_file BASE64_SRC_ROOT simd subdir)
  set(_var "COMPILE_FLAGS_${simd}")
  if (NOT DEFINED ${_var})
    return()
  endif()
  set(_flags "${${_var}}")
  if (_flags STREQUAL " ")
    return()
  endif()
  set(_codec_c "${BASE64_SRC_ROOT}/lib/arch/${subdir}/codec.c")
  if (EXISTS "${_codec_c}")
    set_source_files_properties("${_codec_c}" PROPERTIES COMPILE_FLAGS "${_flags}")
  endif()
endfunction()

# aklomp/base64 sets per-file COMPILE_FLAGS in its subdirectory; those properties are not
# visible when the same absolute paths are added to package static libs at the top level.
# Re-apply SIMD flags using the same compiler settings as third_party/aklomp-base64.
function(_ydb_sdk_apply_aklomp_base64_simd_file_flags BASE64_SRC_ROOT)
  if (NOT IS_DIRECTORY "${BASE64_SRC_ROOT}")
    return()
  endif()
  if (NOT EXISTS "${BASE64_SRC_ROOT}/cmake/Modules/TargetSIMDInstructionSet.cmake")
    return()
  endif()
  include("${BASE64_SRC_ROOT}/cmake/Modules/TargetSIMDInstructionSet.cmake")
  define_SIMD_compile_flags()
  _ydb_sdk_b64_set_codec_file("${BASE64_SRC_ROOT}" SSSE3 ssse3)
  _ydb_sdk_b64_set_codec_file("${BASE64_SRC_ROOT}" SSE41 sse41)
  _ydb_sdk_b64_set_codec_file("${BASE64_SRC_ROOT}" SSE42 sse42)
  _ydb_sdk_b64_set_codec_file("${BASE64_SRC_ROOT}" AVX avx)
  _ydb_sdk_b64_set_codec_file("${BASE64_SRC_ROOT}" AVX2 avx2)
  _ydb_sdk_b64_set_codec_file("${BASE64_SRC_ROOT}" AVX512 avx512)
endfunction()

function(_ydb_sdk_collect_package_target PackageName Tgt)
  if (NOT TARGET ${Tgt})
    return()
  endif()

  get_target_property(aliased_target ${Tgt} ALIASED_TARGET)
  if (aliased_target)
    set(Tgt ${aliased_target})
  endif()

  get_target_property(imported_target ${Tgt} IMPORTED)
  if (imported_target)
    set_property(GLOBAL APPEND PROPERTY YDB_CPP_${PackageName}_PUBLIC_DEPS ${Tgt})
    return()
  endif()

  get_property(visited_targets GLOBAL PROPERTY YDB_CPP_${PackageName}_VISITED_TARGETS)
  if (Tgt IN_LIST visited_targets)
    return()
  endif()
  set_property(GLOBAL APPEND PROPERTY YDB_CPP_${PackageName}_VISITED_TARGETS ${Tgt})
  set_property(GLOBAL APPEND PROPERTY YDB_CPP_${PackageName}_INTERNAL_DEPS ${Tgt})

  get_target_property(tgt_type ${Tgt} TYPE)
  if (tgt_type STREQUAL "STATIC_LIBRARY" OR tgt_type STREQUAL "OBJECT_LIBRARY")
    get_target_property(tgt_srcs ${Tgt} SOURCES)
    get_target_property(tgt_source_dir ${Tgt} SOURCE_DIR)
    if (tgt_srcs)
      set(resolved_srcs "")
      foreach(src IN LISTS tgt_srcs)
        if (NOT src MATCHES "^\\$<")
          if (NOT IS_ABSOLUTE "${src}")
            set(src "${tgt_source_dir}/${src}")
          endif()
          list(APPEND resolved_srcs "${src}")
        endif()
      endforeach()
      if (resolved_srcs)
        get_target_property(tgt_compile_defs ${Tgt} COMPILE_DEFINITIONS)
        get_target_property(tgt_compile_opts ${Tgt} COMPILE_OPTIONS)
        foreach(resolved_src IN LISTS resolved_srcs)
          get_source_file_property(_file_simd_flags ${resolved_src} COMPILE_FLAGS)
          if (_file_simd_flags STREQUAL "NOTFOUND")
            set(_file_simd_flags "")
          endif()
          get_source_file_property(_file_defs ${resolved_src} COMPILE_DEFINITIONS)
          if (_file_defs STREQUAL "NOTFOUND")
            set(_file_defs "")
          endif()

          if (tgt_compile_defs)
            if (_file_defs)
              set_source_files_properties(${resolved_src} PROPERTIES
                COMPILE_DEFINITIONS "${_file_defs};${tgt_compile_defs}")
            else()
              set_source_files_properties(${resolved_src} PROPERTIES
                COMPILE_DEFINITIONS "${tgt_compile_defs}")
            endif()
          elseif (_file_defs)
            set_source_files_properties(${resolved_src} PROPERTIES
              COMPILE_DEFINITIONS "${_file_defs}")
          endif()

          if (tgt_compile_opts)
            set_source_files_properties(${resolved_src} PROPERTIES COMPILE_OPTIONS "${tgt_compile_opts}")
          endif()

          if (_file_simd_flags)
            set_source_files_properties(${resolved_src} PROPERTIES COMPILE_FLAGS "${_file_simd_flags}")
          endif()
        endforeach()
        set_property(GLOBAL APPEND PROPERTY YDB_CPP_${PackageName}_SOURCES ${resolved_srcs})
      endif()
    endif()
  endif()

  foreach(prop IN ITEMS INCLUDE_DIRECTORIES INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(target_include_dirs ${Tgt} ${prop})
    if (target_include_dirs)
      set_property(GLOBAL APPEND PROPERTY YDB_CPP_${PackageName}_INCLUDE_DIRS ${target_include_dirs})
    endif()
  endforeach()

  foreach(prop IN ITEMS LINK_LIBRARIES INTERFACE_LINK_LIBRARIES)
    get_target_property(target_deps ${Tgt} ${prop})
    if (target_deps)
      foreach(dep IN LISTS target_deps)
        if (dep MATCHES "^\\$<")
          continue()
        endif()
        if (TARGET ${dep})
          _ydb_sdk_collect_package_target(${PackageName} ${dep})
        else()
          set_property(GLOBAL APPEND PROPERTY YDB_CPP_${PackageName}_PUBLIC_DEPS ${dep})
        endif()
      endforeach()
    endif()
  endforeach()

  get_target_property(tgt_pub_opts ${Tgt} INTERFACE_COMPILE_OPTIONS)
  if (tgt_pub_opts)
    set_property(GLOBAL APPEND PROPERTY YDB_CPP_${PackageName}_PUBLIC_OPTS ${tgt_pub_opts})
  endif()

  get_target_property(tgt_pub_defs ${Tgt} INTERFACE_COMPILE_DEFINITIONS)
  if (tgt_pub_defs)
    set_property(GLOBAL APPEND PROPERTY YDB_CPP_${PackageName}_PUBLIC_DEFS ${tgt_pub_defs})
  endif()
endfunction()

function(_ydb_sdk_collect_core_target Tgt)
  _ydb_sdk_collect_package_target(CORE ${Tgt})
endfunction()

function(_ydb_sdk_validate_public_headers)
  file(GLOB_RECURSE allHeaders RELATIVE ${YDB_SDK_SOURCE_DIR}/include ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/*)
  file(STRINGS ${YDB_SDK_SOURCE_DIR}/cmake/public_headers.txt specialHeaders)
  file(STRINGS ${YDB_SDK_SOURCE_DIR}/cmake/protos_public_headers.txt protosHeaders)
  if (NOT MSVC)
    list(REMOVE_ITEM specialHeaders library/cpp/deprecated/atomic/atomic_win.h)
  endif()
  list(APPEND allHeaders ${specialHeaders})
  file(COPY ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk DESTINATION ${YDB_SDK_BINARY_DIR}/__validate_headers_dir/include)
  foreach(path ${specialHeaders})
    get_filename_component(relPath ${path} DIRECTORY)
    file(COPY ${YDB_SDK_SOURCE_DIR}/${path}
      DESTINATION ${YDB_SDK_BINARY_DIR}/__validate_headers_dir/include/${relPath}
    )
  endforeach()

  add_custom_target(make_validate_proto_headers
    COMMAND ${CMAKE_COMMAND} -E
    WORKING_DIRECTORY ${YDB_SDK_BINARY_DIR}
  )
  foreach(path ${protosHeaders})
    get_filename_component(relPath ${path} DIRECTORY)
    add_custom_command(OUTPUT ${YDB_SDK_BINARY_DIR}/__validate_headers_dir/include/${path}
      COMMAND ${CMAKE_COMMAND} -E
        copy "${path}" "__validate_headers_dir/include/${relPath}"
      DEPENDS "${path}"
      WORKING_DIRECTORY ${YDB_SDK_BINARY_DIR}
    )
  endforeach()

  list(REMOVE_ITEM allHeaders
    library/cpp/threading/future/core/future-inl.h
    library/cpp/threading/future/wait/wait-inl.h
    library/cpp/yt/misc/guid-inl.h
  )

  set(targetHeaders ${allHeaders})
  list(APPEND targetHeaders ${protosHeaders})
  list(TRANSFORM targetHeaders PREPEND "${YDB_SDK_BINARY_DIR}/__validate_headers_dir/include/")

  list(TRANSFORM allHeaders PREPEND "#include <")
  list(TRANSFORM allHeaders APPEND ">")
  list(JOIN allHeaders "\n" fileContent)

  file(WRITE ${YDB_SDK_BINARY_DIR}/__validate_headers_dir/main.cpp ${fileContent})

  add_library(validate_public_interface MODULE
    ${YDB_SDK_BINARY_DIR}/__validate_headers_dir/main.cpp
    ${targetHeaders}
  )
  target_include_directories(validate_public_interface PUBLIC ${YDB_SDK_BINARY_DIR}/__validate_headers_dir/include)
endfunction()

