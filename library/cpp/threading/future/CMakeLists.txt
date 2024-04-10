add_library(cpp-threading-future)

target_link_libraries(cpp-threading-future PUBLIC
  yutil
)

target_sources(cpp-threading-future PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/future/async_semaphore.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/future/async.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/future/core/future.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/future/core/fwd.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/future/fwd.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/future/wait/fwd.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/future/wait/wait.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/future/wait/wait_group.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/future/wait/wait_policy.cpp
)
