add_library(cw_api3d_cadwork_adapter INTERFACE)

target_include_directories(cw_api3d_cadwork_adapter INTERFACE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(cw_api3d_cadwork_adapter
        INTERFACE cw_api3d_ports
        INTERFACE CwAPI3D::CwAPI3D
)

target_compile_features(cw_api3d_cadwork_adapter INTERFACE cxx_std_23)
