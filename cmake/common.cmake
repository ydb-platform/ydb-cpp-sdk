# Set of common macros

find_package(Python3 REQUIRED)

# assumes ToolName is always both the binary and the target name
function(get_built_tool_path OutBinPath SrcPath ToolName)
  set(${OutBinPath} "${CMAKE_BINARY_DIR}/${SrcPath}/${ToolName}${CMAKE_EXECUTABLE_SUFFIX}" PARENT_SCOPE)
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
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/scripts/run_tool.py -- ${RAGEL_BIN} ${RAGEL_FLAGS} ${ARGN} -o ${CMAKE_CURRENT_BINARY_DIR}/${OutPath} ${Src}
    DEPENDS ${CMAKE_SOURCE_DIR}/scripts/run_tool.py ${Src}
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
      COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/scripts/run_tool.py -- ${YASM_BIN} ${YASM_FLAGS} ${ARGN} -o ${CMAKE_CURRENT_BINARY_DIR}/${OutPath} ${Src}
    DEPENDS ${CMAKE_SOURCE_DIR}/scripts/run_tool.py ${Src}
  )
  target_sources(${TgtName} ${Key} ${CMAKE_CURRENT_BINARY_DIR}/${OutPath})
endfunction()

function(target_joined_source TgtName Out)
  foreach(InSrc ${ARGN})
    file(RELATIVE_PATH IncludePath ${CMAKE_SOURCE_DIR} ${InSrc})
    list(APPEND IncludesList ${IncludePath})
  endforeach()
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${Out}
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/scripts/gen_join_srcs.py ${CMAKE_CURRENT_BINARY_DIR}/${Out} ${IncludesList}
    DEPENDS ${CMAKE_SOURCE_DIR}/scripts/gen_join_srcs.py ${ARGN}
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
    file(RELATIVE_PATH SrcRealPath ${CMAKE_SOURCE_DIR} ${Src})
    get_filename_component(SrcDir ${SrcRealPath} DIRECTORY)
    get_filename_component(SrcName ${SrcRealPath} NAME_WLE)
    get_filename_component(SrcExt ${SrcRealPath} LAST_EXT)
    set(SrcCopy "${CMAKE_BINARY_DIR}/${SrcDir}/${SrcName}${CompileOutSuffix}${SrcExt}")
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
      -I${CMAKE_SOURCE_DIR}/${SrcDir}
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


if (MSVC AND (${CMAKE_VERSION} VERSION_LESS "3.21.0"))
    message(FATAL_ERROR "Build with MSVC-compatible toolchain requires at least cmake 3.21.0 because of used TARGET_OBJECTS feature")
endif()

function(add_global_library_for TgtName MainName)
  if (MSVC)
    add_library(${TgtName} OBJECT ${ARGN})
    add_dependencies(${TgtName} ${MainName}) # needed because object library can use some extra generated files in MainName
    target_link_libraries(${MainName} INTERFACE ${TgtName} "$<TARGET_OBJECTS:${TgtName}>")
  else()
    add_library(${TgtName} STATIC ${ARGN})
    add_library(${TgtName}.wholearchive INTERFACE)
    add_dependencies(${TgtName}.wholearchive ${TgtName})
    add_dependencies(${TgtName} ${MainName})
    if(APPLE)
      target_link_options(${TgtName}.wholearchive INTERFACE "SHELL:-Wl,-force_load,$<TARGET_FILE:${TgtName}>")
    else()
      target_link_options(${TgtName}.wholearchive INTERFACE "SHELL:-Wl,--whole-archive $<TARGET_FILE:${TgtName}> -Wl,--no-whole-archive")
    endif()
    target_link_libraries(${MainName} INTERFACE ${TgtName}.wholearchive)
  endif()
endfunction()

function(vcs_info Tgt)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/vcs_info.json
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/scripts/generate_vcs_info.py ${CMAKE_CURRENT_BINARY_DIR}/vcs_info.json ${CMAKE_SOURCE_DIR}
    DEPENDS ${CMAKE_SOURCE_DIR}/scripts/generate_vcs_info.py
  )

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/__vcs_version__.c
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/scripts/vcs_info.py ${CMAKE_CURRENT_BINARY_DIR}/vcs_info.json ${CMAKE_CURRENT_BINARY_DIR}/__vcs_version__.c ${CMAKE_SOURCE_DIR}/scripts/c_templates/svn_interface.c
    DEPENDS ${CMAKE_SOURCE_DIR}/scripts/vcs_info.py ${CMAKE_SOURCE_DIR}/scripts/c_templates/svn_interface.c ${CMAKE_CURRENT_BINARY_DIR}/vcs_info.json
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

  get_built_tool_path(rescompiler_bin tools/rescompiler/bin rescompiler)

  add_custom_command(
    OUTPUT ${Output}
    COMMAND ${rescompiler_bin} ${Output} ${ResourcesList}
    DEPENDS ${RESOURCE_ARGS_INPUTS} ${rescompiler_bin}
  )
endfunction()
