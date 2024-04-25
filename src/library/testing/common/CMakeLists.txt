add_library(cpp-testing-common)
<<<<<<< HEAD
<<<<<<< HEAD

=======
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======

>>>>>>> origin/main
target_link_libraries(cpp-testing-common PUBLIC
  yutil
  library-cpp-json
  library-cpp-svnversion
)
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> origin/main

target_sources(cpp-testing-common PRIVATE
  env.cpp
  env_var.cpp
  network.cpp
  probe.cpp
  scope.cpp
)

vcs_info(cpp-testing-common)
<<<<<<< HEAD
=======
target_sources(cpp-testing-common PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/common/env.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/common/env_var.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/common/network.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/common/probe.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/common/scope.cpp
)

vcs_info(cpp-testing-common)

add_subdirectory(ut)
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
