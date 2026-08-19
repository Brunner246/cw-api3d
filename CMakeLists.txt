cmake_minimum_required(VERSION 3.28)

project(cw_api3d
    VERSION 0.1.0
    LANGUAGES CXX C
)

include(cmake/configuration.cmake)
include(cmake/CwAPI3D.cmake)

find_package(spdlog CONFIG REQUIRED)

add_subdirectory(src/ports)
add_subdirectory(src/application)
add_subdirectory(src/adapters/driven/logging)
add_subdirectory(src/adapters/driven/cadwork)

enable_testing()
add_subdirectory(tests)
