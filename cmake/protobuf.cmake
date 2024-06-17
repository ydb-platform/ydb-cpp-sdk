function(_ydb_sdk_gen_proto_messages TGT Scope)
  get_property(ProtocExtraOutsSuf TARGET ${TGT} PROPERTY PROTOC_EXTRA_OUTS)
  foreach(proto ${ARGN})
    if(proto MATCHES ${YDB_SDK_BINARY_DIR})
      file(RELATIVE_PATH protoRel ${YDB_SDK_BINARY_DIR} ${proto})
    elseif (proto MATCHES ${YDB_SDK_SOURCE_DIR})
      file(RELATIVE_PATH protoRel ${YDB_SDK_SOURCE_DIR} ${proto})
    else()
      set(protoRel ${proto})
    endif()
    get_filename_component(OutputBase ${protoRel} NAME_WLE)
    get_filename_component(OutputDir ${YDB_SDK_BINARY_DIR}/${protoRel} DIRECTORY)
    list(TRANSFORM ProtocExtraOutsSuf PREPEND ${OutputDir}/${OutputBase} OUTPUT_VARIABLE ProtocExtraOuts)
    add_custom_command(
      OUTPUT
        ${OutputDir}/${OutputBase}.pb.cc
        ${OutputDir}/${OutputBase}.pb.h
        ${ProtocExtraOuts}
      COMMAND protobuf::protoc
        ${COMMON_PROTOC_FLAGS}
        "-I$<JOIN:$<TARGET_GENEX_EVAL:${TGT},$<TARGET_PROPERTY:${TGT},PROTO_ADDINCL>>,;-I>"
        "$<JOIN:$<TARGET_GENEX_EVAL:${TGT},$<TARGET_PROPERTY:${TGT},PROTO_OUTS>>,;>"
        "$<JOIN:$<TARGET_GENEX_EVAL:${TGT},$<TARGET_PROPERTY:${TGT},PROTOC_OPTS>>,;>"
        "--experimental_allow_proto3_optional"
        ${protoRel}
      DEPENDS
        ${proto}
      WORKING_DIRECTORY ${YDB_SDK_SOURCE_DIR}
      COMMAND_EXPAND_LISTS
    )
    target_sources(${TGT} ${Scope}
      ${OutputDir}/${OutputBase}.pb.cc ${OutputDir}/${OutputBase}.pb.h
      ${ProtocExtraOuts}
    )
  endforeach()
endfunction()

function(_ydb_sdk_init_proto_library_impl TGT USE_API_COMMON_PROTOS)
  add_library(${TGT})
  target_link_libraries(${TGT}
    protobuf::libprotobuf
  )

  set(proto_incls ${YDB_SDK_SOURCE_DIR})

  if (USE_API_COMMON_PROTOS)
    target_link_libraries(${TGT}
      api-common-protos
    )
    list(APPEND proto_incls ${YDB_SDK_SOURCE_DIR}/third_party/api-common-protos)
  endif()

  set_property(TARGET ${TGT} PROPERTY 
    PROTO_ADDINCL ${proto_incls}
  )

  set_property(TARGET ${TGT} PROPERTY 
    PROTO_OUTS --cpp_out=${YDB_SDK_BINARY_DIR}
  )
endfunction()

function(_ydb_sdk_add_proto_library TGT)
  set(opts "USE_API_COMMON_PROTOS")
  set(multiValueArgs "SOURCES")
  cmake_parse_arguments(
    ARG "${opts}" "${oneValueArgs}" "${multiValueArgs}" "${ARGN}"
  )

  _ydb_sdk_init_proto_library_impl(${TGT} $<BOOL:ARG_USE_API_COMMON_PROTOS>)
  _ydb_sdk_gen_proto_messages(${TGT} PRIVATE ${ARG_SOURCES})
endfunction()

function(_ydb_sdk_add_grpc_library TGT)
  set(opts "USE_API_COMMON_PROTOS")
  set(multiValueArgs "SOURCES")
  cmake_parse_arguments(
    ARG "${opts}" "${oneValueArgs}" "${multiValueArgs}" "${ARGN}"
  )

  _ydb_sdk_init_proto_library_impl(${TGT} $<BOOL:ARG_USE_API_COMMON_PROTOS>)

  target_link_libraries(${TGT}
    gRPC::grpc++
  )

  set_property(TARGET ${TGT} APPEND PROPERTY
    PROTOC_OPTS --grpc_cpp_out=${YDB_SDK_BINARY_DIR}/$<TARGET_PROPERTY:${TGT},PROTO_NAMESPACE> --plugin=protoc-gen-grpc_cpp=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
  )

  set_property(TARGET ${TGT} APPEND PROPERTY
    PROTOC_DEPS gRPC::grpc_cpp_plugin
  )

  _ydb_sdk_gen_proto_messages(${TGT} PRIVATE ${ARG_SOURCES})
endfunction()
