add_library(cw_api3d_ports INTERFACE)

target_include_directories(cw_api3d_ports INTERFACE
        $<BUILD_INTERFACE:${CW_API3D_KIT_ROOT}>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(cw_api3d_ports INTERFACE cxx_std_23)
