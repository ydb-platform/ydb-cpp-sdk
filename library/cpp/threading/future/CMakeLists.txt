add_library(cpp-threading-future)

target_link_libraries(cpp-threading-future PUBLIC
  contrib-libs-cxxsupp
)

target_sources(cpp-threading-future PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/threading/future/async_semaphore.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/threading/future/async.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/threading/future/core/future.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/threading/future/core/fwd.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/threading/future/fwd.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/threading/future/wait/fwd.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/threading/future/wait/wait.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/threading/future/wait/wait_group.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/threading/future/wait/wait_policy.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-threading-future PUBLIC
    contrib-libs-linux-headers
  )
endif()
