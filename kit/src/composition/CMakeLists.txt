add_library(cw_api3d_composition STATIC
        Bootstrapping.cpp
        Bootstrapping.h
)

target_link_libraries(cw_api3d_composition PUBLIC
        cw_api3d_ports
        cw_api3d_application
        cw_api3d_spdlog_adapter
        cw_api3d_cadwork_adapter
        spdlog::spdlog
        CwAPI3D::CwAPI3D
)

target_include_directories(cw_api3d_composition PUBLIC
        ${CW_API3D_KIT_ROOT}
)

target_compile_features(cw_api3d_composition PUBLIC cxx_std_23)
