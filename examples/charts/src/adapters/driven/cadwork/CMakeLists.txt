add_library(cw_api3d_charts_cadwork
        CadworkElementCatalogAdapter.cpp
        CadworkElementCatalogAdapter.h
        ElementCatalogAdapter.h
        CadworkElementActivationAdapter.cpp
        CadworkElementActivationAdapter.h
        ElementActivationAdapter.h
)

target_include_directories(cw_api3d_charts_cadwork PUBLIC
        $<BUILD_INTERFACE:${CW_API3D_CHARTS_ROOT}>
        $<BUILD_INTERFACE:${CW_API3D_KIT_ROOT}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(cw_api3d_charts_cadwork
        PUBLIC cw_api3d_charts_ports
        PUBLIC cw_api3d_ports
        PUBLIC CwAPI3D::CwAPI3D
)

target_compile_features(cw_api3d_charts_cadwork PUBLIC cxx_std_23)
