add_library(service-pages-tablesorter INTERFACE)

target_link_libraries(service-pages-tablesorter INTERFACE
  yutil
  library-cpp-resource
  cpp-monlib-dynamic_counters
)

add_global_library_for(service-pages-tablesorter.global service-pages-tablesorter)

target_link_libraries(service-pages-tablesorter.global PUBLIC
  yutil
  library-cpp-resource
  cpp-monlib-dynamic_counters
)

target_sources(service-pages-tablesorter.global PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/monlib/service/pages/tablesorter/CMakeLists.txt
=======
>>>>>>> origin/main
  ${YDB_SDK_BINARY_DIR}/src/library/monlib/service/pages/tablesorter/0c255888afcc3e2adc5fd360ee152bac.cpp
)

resources(service-pages-tablesorter.global
  ${YDB_SDK_BINARY_DIR}/src/library/monlib/service/pages/tablesorter/0c255888afcc3e2adc5fd360ee152bac.cpp
  INPUTS
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/pages/tablesorter/resources/jquery.tablesorter.css
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/pages/tablesorter/resources/jquery.tablesorter.js
<<<<<<< HEAD
=======
  ${YDB_SDK_BINARY_DIR}/library/cpp/monlib/service/pages/tablesorter/0c255888afcc3e2adc5fd360ee152bac.cpp
<<<<<<< HEAD
<<<<<<< HEAD
=======
  ${YDB_SDK_BINARY_DIR}/src/library/monlib/service/pages/tablesorter/0c255888afcc3e2adc5fd360ee152bac.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)

resources(service-pages-tablesorter.global
  ${YDB_SDK_BINARY_DIR}/src/library/monlib/service/pages/tablesorter/0c255888afcc3e2adc5fd360ee152bac.cpp
  INPUTS
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/service/pages/tablesorter/resources/jquery.tablesorter.css
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/service/pages/tablesorter/resources/jquery.tablesorter.js
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/service/pages/tablesorter/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/pages/tablesorter/resources/jquery.tablesorter.css
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/pages/tablesorter/resources/jquery.tablesorter.js
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
=======
=======
  ${YDB_SDK_BINARY_DIR}/src/library/monlib/service/pages/tablesorter/0c255888afcc3e2adc5fd360ee152bac.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)

resources(service-pages-tablesorter.global
  ${YDB_SDK_BINARY_DIR}/src/library/monlib/service/pages/tablesorter/0c255888afcc3e2adc5fd360ee152bac.cpp
  INPUTS
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/service/pages/tablesorter/resources/jquery.tablesorter.css
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/service/pages/tablesorter/resources/jquery.tablesorter.js
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/service/pages/tablesorter/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/pages/tablesorter/resources/jquery.tablesorter.css
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/pages/tablesorter/resources/jquery.tablesorter.js
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
  KEYS
  jquery.tablesorter.css
  jquery.tablesorter.js
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(service-pages-tablesorter INTERFACE
    
  )
  target_link_libraries(service-pages-tablesorter.global PUBLIC
    
  )
endif()
