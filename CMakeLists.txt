cmake_minimum_required(VERSION 3.28)

project(cw_api3d
    VERSION 0.1.0
    LANGUAGES CXX C
)

include(cmake/configuration.cmake)
include(cmake/CwAPI3D.cmake)
include(cmake/cadwork_deploy.cmake)

find_package(spdlog CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

if(CUSTOM_QT_PATH)
    list(PREPEND CMAKE_PREFIX_PATH "${CUSTOM_QT_PATH}")
endif()

enable_testing()

add_subdirectory(kit)
add_subdirectory(examples/hello-plugin)
add_subdirectory(examples/charts)
