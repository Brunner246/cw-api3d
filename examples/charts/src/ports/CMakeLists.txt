add_library(cw_api3d_charts_ports INTERFACE)

target_include_directories(cw_api3d_charts_ports INTERFACE
        $<BUILD_INTERFACE:${CW_API3D_CHARTS_ROOT}>
        $<BUILD_INTERFACE:${CW_API3D_KIT_ROOT}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(cw_api3d_charts_ports INTERFACE
        cw_api3d_ports
)

target_compile_features(cw_api3d_charts_ports INTERFACE cxx_std_23)
