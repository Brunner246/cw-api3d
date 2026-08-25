set(CW_API3D_KIT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "Root of the cw-api3d kit sources")

add_subdirectory(src/ports)
add_subdirectory(src/application)
add_subdirectory(src/adapters/driven/logging)
add_subdirectory(src/adapters/driven/cadwork)
add_subdirectory(src/composition)
add_subdirectory(tests)
