include(DownloadUsingCPM)

if(NOT "${CMAKE_BINARY_DIR}/${STUB_DIR}" IN_LIST CMAKE_PREFIX_PATH)
        set(CMAKE_PREFIX_PATH
            "${CMAKE_BINARY_DIR}/${STUB_DIR}" ${CMAKE_PREFIX_PATH}
        )
endif()

# todo сделать нормально без этих страшных путей
include(${CMAKE_CURRENT_LIST_DIR}/dependencies/SetupBrotli.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/dependencies/SetupBase64.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/dependencies/SetupAbseil.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/dependencies/SetupJwtCpp.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/dependencies/SetupProtobuf.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/dependencies/SetupGrpc.cmake)
