_ydb_sdk_add_library(threading-future-subscription)

target_link_libraries(threading-future-subscription PUBLIC
  yutil
  threading-future
)

target_sources(threading-future-subscription 
  PRIVATE
    subscription.cpp
    wait_all.cpp
    wait_all_or_exception.cpp
    wait_any.cpp
)

_ydb_sdk_install_targets(TARGETS threading-future-subscription)

if (YDB_SDK_TESTS)
  add_ydb_test(NAME future-subscription-ut
    SOURCES
      subscription_ut.cpp
      wait_all_ut.cpp
      wait_all_or_exception_ut.cpp
      wait_any_ut.cpp
      wait_ut_common.cpp
    LINK_LIBRARIES
      threading-future-subscription
    LABELS
      unit
  )
endif()
