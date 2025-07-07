add_executable(topic_writer_transaction)

target_link_libraries(topic_writer_transaction
  PUBLIC
    yutil
    YDB-CPP-SDK::Topic
    YDB-CPP-SDK::Query
)

target_sources(topic_writer_transaction
  PRIVATE
    main.cpp
)

vcs_info(topic_writer_transaction)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  target_link_libraries(topic_writer_transaction PUBLIC
    cpuid_check
  )
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(topic_writer_transaction PRIVATE
    -ldl
    -lrt
    -Wl,--no-as-needed
    -lpthread
  )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_link_options(topic_writer_transaction PRIVATE
    -Wl,-platform_version,macos,11.0,11.0
    -framework
    CoreFoundation
  )
endif()
