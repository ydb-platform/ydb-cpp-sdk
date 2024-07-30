add_library(getopt-small)
target_link_libraries(getopt-small
  PUBLIC
    yutil
    colorizer
)
target_sources(getopt-small PRIVATE
  small/completer.cpp
  small/completer_command.cpp
  small/completion_generator.cpp
  small/formatted_output.cpp
  small/last_getopt.cpp
  small/last_getopt_easy_setup.cpp
  small/last_getopt_opt.cpp
  small/last_getopt_opts.cpp
  small/last_getopt_parser.cpp
  small/last_getopt_parse_result.cpp
  small/modchooser.cpp
  small/opt.cpp
  small/opt2.cpp
  small/posix_getopt.cpp
  small/wrap.cpp
  small/ygetopt.cpp
)
_ydb_sdk_install_targets(TARGETS getopt-small)


add_library(getopt INTERFACE)
add_global_library_for(getopt.global getopt)
target_link_libraries(getopt.global
  PUBLIC
    yutil
    getopt-small
  PRIVATE
    svnversion
    build_info
)
target_sources(getopt.global PRIVATE
  print.cpp
)
