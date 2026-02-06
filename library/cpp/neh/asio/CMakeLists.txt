_ydb_sdk_add_library(neh-asio)

target_sources(neh-asio
  PRIVATE
    asio.cpp
    deadline_timer_impl.cpp
    executor.cpp
    io_service_impl.cpp
    poll_interrupter.cpp
    tcp_acceptor_impl.cpp
    tcp_socket_impl.cpp
)

target_link_libraries(neh-asio
  PUBLIC
    yutil
    contrib-libs-libc_compat
    coroutine-engine
    dns
  PRIVATE
    enum_serialization_runtime
)

_ydb_sdk_install_targets(TARGETS neh-asio)