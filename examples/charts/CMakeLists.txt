set(CW_API3D_CHARTS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(src/ports)
add_subdirectory(src/application)
add_subdirectory(src/adapters/driven/cadwork)
add_subdirectory(src/adapters/driving)
add_subdirectory(src/composition)
add_subdirectory(tests)
