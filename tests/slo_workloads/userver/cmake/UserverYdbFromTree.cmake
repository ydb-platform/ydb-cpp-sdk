function(userver_ydb_from_tree)
    if(TARGET userver::ydb)
        return()
    endif()

    if(NOT TARGET userver-core AND TARGET userver::core)
        add_library(userver-core ALIAS userver::core)
    endif()

    if(NOT USERVER_ROOT_DIR)
        if(DEFINED ENV{USERVER_SOURCE_DIR})
            set(USERVER_ROOT_DIR "$ENV{USERVER_SOURCE_DIR}" CACHE PATH "" FORCE)
        else()
            message(FATAL_ERROR "USERVER_SOURCE_DIR must point to a userver source tree (matching the installed .deb version)")
        endif()
    endif()

    if(NOT EXISTS "${USERVER_ROOT_DIR}/ydb/CMakeLists.txt")
        message(FATAL_ERROR "userver ydb sources not found at ${USERVER_ROOT_DIR}/ydb")
    endif()

    set(USERVER_INSTALL OFF CACHE BOOL "" FORCE)
    set(USERVER_BUILD_TESTS OFF CACHE BOOL "" FORCE)

    # The .deb is built with USERVER_FEATURE_YDB=OFF and omits scripts/chaotic and
    # chaotic-gen-dynamic-configs; use the matching source tree for codegen instead.
    set(USERVER_CMAKE_DIR "${USERVER_ROOT_DIR}/cmake" CACHE PATH "" FORCE)
    set(USERVER_CHAOTIC_SCRIPTS_PATH "${USERVER_ROOT_DIR}/scripts/chaotic" CACHE PATH "" FORCE)

    list(PREPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_LIST_DIR}/cmake"
        "${USERVER_ROOT_DIR}/cmake"
    )
    list(PREPEND CMAKE_PROGRAM_PATH
        "${USERVER_ROOT_DIR}/chaotic/bin-dynamic-configs"
        "${USERVER_ROOT_DIR}/chaotic/bin"
    )

    include(${USERVER_ROOT_DIR}/cmake/PrepareInstall.cmake)
    include(${USERVER_ROOT_DIR}/cmake/UserverCodegenTarget.cmake)
    include(${USERVER_ROOT_DIR}/cmake/UserverModule.cmake)
    include(${USERVER_ROOT_DIR}/cmake/ChaoticGen.cmake)

    set(_userver_chaotic_dynamic_configs_bin
        "${USERVER_ROOT_DIR}/chaotic/bin-dynamic-configs/chaotic-gen-dynamic-configs")
    if(EXISTS "${_userver_chaotic_dynamic_configs_bin}")
        set_property(GLOBAL PROPERTY userver_chaotic_dynamic_configs_bin "${_userver_chaotic_dynamic_configs_bin}")
    endif()

    add_subdirectory(${USERVER_ROOT_DIR}/ydb ${CMAKE_CURRENT_BINARY_DIR}/userver-ydb-build)

    if(TARGET userver-ydb AND NOT TARGET userver::ydb)
        add_library(userver::ydb ALIAS userver-ydb)
    endif()
endfunction()
