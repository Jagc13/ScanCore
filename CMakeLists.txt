cmake_minimum_required(VERSION 3.20)

project(ScanCore
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(scancore_core
    src/DeviceManager.cpp
    src/SerialConnection.cpp
    src/BarcodeCaptureEngine.cpp
    src/ScannerService.cpp
)

target_include_directories(scancore_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

if(WIN32)
    target_link_libraries(scancore_core
        PRIVATE
        setupapi
    )
endif()

add_executable(scancore
    src/main.cpp
)

target_link_libraries(scancore
    PRIVATE
        scancore_core
)

include(CTest)

if(BUILD_TESTING)

    # Task 3 - Barcode capture tests
    add_executable(capture_tests
        tests/BarcodeCaptureEngineTests.cpp
    )

    target_link_libraries(capture_tests
        PRIVATE scancore_core
    )

    add_test(
        NAME capture_tests
        COMMAND capture_tests
    )

    # Task 5 - Asynchronous scanner tests
    add_executable(scanner_service_tests
        tests/ScannerServiceTests.cpp
    )

    target_link_libraries(scanner_service_tests
        PRIVATE scancore_core
    )

    add_test(
        NAME scanner_service_tests
        COMMAND scanner_service_tests
    )

endif()