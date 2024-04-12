add_library(cpp-threading-future)

target_link_libraries(cpp-threading-future PUBLIC
  yutil
)

target_sources(cpp-threading-future PRIVATE
<<<<<<< HEAD
  async_semaphore.cpp
  async.cpp
  core/future.cpp
  wait/wait.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/future/async_semaphore.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/future/async.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/future/core/future.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/future/core/fwd.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/future/fwd.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/future/wait/fwd.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/future/wait/wait.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/future/wait/wait_group.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/future/wait/wait_policy.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)
