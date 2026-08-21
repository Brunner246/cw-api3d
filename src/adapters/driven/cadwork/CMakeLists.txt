add_library(cw_api3d_cadwork_adapter
        CadworkUtilityAdapter.cpp
        CadworkUtilityAdapter.h
        UtilityControllerAdapter.h
)

target_include_directories(cw_api3d_cadwork_adapter PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(cw_api3d_cadwork_adapter
        PUBLIC cw_api3d_ports
        PUBLIC CwAPI3D::CwAPI3D
)

target_compile_features(cw_api3d_cadwork_adapter PUBLIC cxx_std_23)
