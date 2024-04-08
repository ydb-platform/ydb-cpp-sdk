include(common)

function(target_proto_plugin Tgt Name PluginTarget)
  set_property(TARGET ${Tgt} APPEND PROPERTY
    PROTOC_OPTS --${Name}_out=${CMAKE_BINARY_DIR}/$<TARGET_PROPERTY:${Tgt},PROTO_NAMESPACE> --plugin=protoc-gen-${Name}=$<TARGET_FILE:${PluginTarget}>
  )
  set_property(TARGET ${Tgt} APPEND PROPERTY
    PROTOC_DEPS ${PluginTarget}
  )
endfunction()

function(target_proto_addincls Tgt)
  set_property(TARGET ${Tgt} APPEND PROPERTY
    PROTO_ADDINCL ${ARGN}
  )
endfunction()

function(target_proto_outs Tgt)
  set_property(TARGET ${Tgt} APPEND PROPERTY
    PROTO_OUTS ${ARGN}
  )
endfunction()

function(target_proto_messages Tgt Scope)

  get_property(ProtocExtraOutsSuf TARGET ${Tgt} PROPERTY PROTOC_EXTRA_OUTS)
  foreach(proto ${ARGN})
    if(proto MATCHES ${CMAKE_BINARY_DIR})
      file(RELATIVE_PATH protoRel ${CMAKE_BINARY_DIR} ${proto})
    elseif (proto MATCHES ${YDB_SDK_SOURCE_DIR})
      file(RELATIVE_PATH protoRel ${YDB_SDK_SOURCE_DIR} ${proto})
    else()
      set(protoRel ${proto})
    endif()
    get_filename_component(OutputBase ${protoRel} NAME_WLE)
    get_filename_component(OutputDir ${CMAKE_BINARY_DIR}/${protoRel} DIRECTORY)
    list(TRANSFORM ProtocExtraOutsSuf PREPEND ${OutputDir}/${OutputBase} OUTPUT_VARIABLE ProtocExtraOuts)
    add_custom_command(
      OUTPUT
        ${OutputDir}/${OutputBase}.pb.cc
        ${OutputDir}/${OutputBase}.pb.h
        ${ProtocExtraOuts}
      COMMAND protobuf::protoc
        ${COMMON_PROTOC_FLAGS}
        "-I$<JOIN:$<TARGET_GENEX_EVAL:${Tgt},$<TARGET_PROPERTY:${Tgt},PROTO_ADDINCL>>,;-I>"
        "$<JOIN:$<TARGET_GENEX_EVAL:${Tgt},$<TARGET_PROPERTY:${Tgt},PROTO_OUTS>>,;>"
        "$<JOIN:$<TARGET_GENEX_EVAL:${Tgt},$<TARGET_PROPERTY:${Tgt},PROTOC_OPTS>>,;>"
        "--experimental_allow_proto3_optional"
        ${protoRel}
      DEPENDS
        ${proto}
      WORKING_DIRECTORY ${YDB_SDK_SOURCE_DIR}
      COMMAND_EXPAND_LISTS
    )
    target_sources(${Tgt} ${Scope}
      ${OutputDir}/${OutputBase}.pb.cc ${OutputDir}/${OutputBase}.pb.h
      ${ProtocExtraOuts}
    )
  endforeach()
endfunction()
