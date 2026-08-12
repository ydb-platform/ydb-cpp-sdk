add_executable(topic_deferred_publish)

target_link_libraries(topic_deferred_publish PUBLIC
  yutil
  YDB-CPP-SDK::Topic
)

target_sources(topic_deferred_publish PRIVATE
  main.cpp
)

vcs_info(topic_deferred_publish)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  target_link_libraries(topic_deferred_publish PUBLIC
    cpuid_check
  )
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(topic_deferred_publish PRIVATE
    -ldl
    -lrt
    -Wl,--no-as-needed
    -lpthread
  )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_link_options(topic_deferred_publish PRIVATE
    -framework
    CoreFoundation
  )
endif()
