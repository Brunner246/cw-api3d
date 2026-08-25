add_library(cw_api3d_application INTERFACE)

target_include_directories(cw_api3d_application INTERFACE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(cw_api3d_application INTERFACE
        cw_api3d_ports
)

target_compile_features(cw_api3d_application INTERFACE cxx_std_23)
