function(_ydb_sdk_gen_proto_messages Tgt Scope WorkDir Grpc)
  file(RELATIVE_PATH WorkDirRel ${YDB_SDK_SOURCE_DIR} ${WorkDir})
  foreach(ProtoPath ${ARGN})
    get_filename_component(OutputBase ${ProtoPath} NAME_WLE)
    get_target_property(OutputRelPath ${Tgt} PROTOC_PATH_${OutputBase})
    if (${OutputRelPath} STREQUAL "OutputRelPath-NOTFOUND")
      set(OutputRelPath ${WorkDirRel})
    endif()
    if (NOT ${ProtoPath} MATCHES ${WorkDir})
      message(FATAL_ERROR "Proto path '${ProtoPath}' isn't in workdir ${WorkDir}")
    endif()
    file(RELATIVE_PATH ProtoRelPath ${WorkDir} ${ProtoPath})
    get_filename_component(OutputDir ${YDB_SDK_BINARY_DIR}/${OutputRelPath}/${ProtoRelPath} DIRECTORY)
    set(ProtocOuts
      ${OutputDir}/${OutputBase}.pb.cc
      ${OutputDir}/${OutputBase}.pb.h
    )
    set(ProtocOpts "")
    if (Grpc)
      list(APPEND ProtocOuts
        ${OutputDir}/${OutputBase}.grpc.pb.cc
        ${OutputDir}/${OutputBase}.grpc.pb.h
      )
      list(APPEND ProtocOpts
        "--grpc_cpp_out=${YDB_SDK_BINARY_DIR}/$<TARGET_PROPERTY:${Tgt},PROTO_NAMESPACE>" 
        "--plugin=protoc-gen-grpc_cpp=$<TARGET_GENEX_EVAL:${Tgt},$<TARGET_FILE:gRPC::grpc_cpp_plugin>>"
      )
    endif()

    add_custom_command(
      OUTPUT
        ${ProtocOuts}
      COMMAND protobuf::protoc
        ${COMMON_PROTOC_FLAGS}
        "-I$<JOIN:$<TARGET_GENEX_EVAL:${Tgt},$<TARGET_PROPERTY:${Tgt},PROTO_ADDINCL>>,;-I>"
        "-I${WorkDir}"
        "--cpp_out=${YDB_SDK_BINARY_DIR}/$<TARGET_PROPERTY:${Tgt},PROTO_NAMESPACE>"
        "${ProtocOpts}"
        "--experimental_allow_proto3_optional"
        ${OutputRelPath}/${ProtoRelPath}
      DEPENDS
        ${ProtoPath}
      WORKING_DIRECTORY ${YDB_SDK_SOURCE_DIR}
      COMMAND_EXPAND_LISTS
    )
    target_sources(${Tgt} ${Scope}
      ${ProtocOuts}
    )
  endforeach()
endfunction()

function(_ydb_sdk_init_proto_library_impl Tgt USE_API_COMMON_PROTOS)
  add_library(${Tgt})
  target_link_libraries(${Tgt} PUBLIC
    protobuf::libprotobuf
  )

  set(proto_incls ${YDB_SDK_SOURCE_DIR})

  if (USE_API_COMMON_PROTOS)
    target_link_libraries(${Tgt} PUBLIC
      api-common-protos
    )
    list(APPEND proto_incls ${YDB_SDK_SOURCE_DIR}/third_party/api-common-protos)
  endif()

  set_property(TARGET ${Tgt} PROPERTY 
    PROTO_ADDINCL ${proto_incls}
  )

  foreach(ProtoPath ${ARGN})
    get_filename_component(OutputBase ${ProtoPath} NAME_WLE)
    get_filename_component(OutputDir ydb-cpp-sdk/${ProtoPath} DIRECTORY)
    set_property(TARGET ${Tgt} PROPERTY
      PROTOC_HEADER_PATH_${OutputBase} ${OutputDir}
    )
  endforeach()
endfunction()

function(_ydb_sdk_add_proto_library Tgt)
  set(opts "USE_API_COMMON_PROTOS" "GRPC")
  set(oneValueArgs "WORKDIR")
  set(multiValueArgs "SOURCES" "PUBLIC_PROTOS")
  cmake_parse_arguments(
    ARG "${opts}" "${oneValueArgs}" "${multiValueArgs}" "${ARGN}"
  )
  if (NOT WORKDIR IN_LIST ARG_UNPARSED_ARGUMENTS)
    set(ARG_WORKDIR ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  _ydb_sdk_init_proto_library_impl(${Tgt} $<BOOL:ARG_USE_API_COMMON_PROTOS> ${ARG_PUBLIC_PROTOS})

  if (ARG_GRPC)
    target_link_libraries(${Tgt} PUBLIC
      gRPC::grpc++
    )
  endif()

  _ydb_sdk_gen_proto_messages(${Tgt} PRIVATE ${ARG_WORKDIR} $<BOOL:ARG_GRPC> ${ARG_SOURCES})
endfunction()

function(_ydb_sdk_install_proto_headers Tgt ProtoNames)
  get_target_property(ProtocExtraOutsSuf ${Tgt} PROTOC_EXTRA_OUTS)
  set(HeaderPaths "")
  foreach(Proto ${ProtoNames})
    get_target_property(NewPath ${Tgt} PROTOC_HEADER_PATH_${Proto})
    if (${NewPath} STREQUAL "NewPath-NOTFOUND")
      message(FATAL_ERROR "Cannot install proto header '${Proto}' because it wasn't marked as public")
    endif()
    list(APPEND HeaderPaths
      ${YDB_SDK_BINARY_DIR}/ydb-cpp-sdk/${NewPath}/${Proto}.pb.h
    )
    if (NOT ${ProtocExtraOutsSuf} STREQUAL "ProtocExtraOutsSuf-NOTFOUND")
      list(APPEND HeaderPaths
        ${YDB_SDK_BINARY_DIR}/ydb-cpp-sdk/${NewPath}/${Proto}.${ProtocExtraOutsSuf}.h
      )
    endif()
  endforeach()
  _ydb_sdk_directory_install(FILES
    ${HeaderPaths}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${NewPath}
  )
endfunction()
