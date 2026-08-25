add_library(cw_api3d_charts_composition STATIC
        ChartsBootstrap.cpp
        ChartsBootstrap.h
        HostAbsentQtRuntimeVisibility.cpp
        HostAbsentQtRuntimeVisibility.h
        IStatisticsPanel.h
        PluginUiSession.cpp
        PluginUiSession.h
)

target_link_libraries(cw_api3d_charts_composition PUBLIC
        cw_api3d_composition
        cw_api3d_ports
        cw_api3d_charts_ports
        cw_api3d_charts_application
        cw_api3d_charts_cadwork
        cw_api3d_spdlog_adapter
        cw_api3d_cadwork_adapter
        spdlog::spdlog
        CwAPI3D::CwAPI3D
)

target_include_directories(cw_api3d_charts_composition PUBLIC
        ${CW_API3D_CHARTS_ROOT}
        ${CW_API3D_KIT_ROOT}
)

target_compile_features(cw_api3d_charts_composition PUBLIC cxx_std_23)

add_library(cw_api3d_charts SHARED
        PluginEntry.cpp
)

target_link_libraries(cw_api3d_charts PRIVATE
        cw_api3d_charts_composition
)

target_include_directories(cw_api3d_charts PRIVATE
        ${CW_API3D_CHARTS_ROOT}
        ${CW_API3D_KIT_ROOT}
)

target_compile_features(cw_api3d_charts PRIVATE cxx_std_23)

target_compile_definitions(cw_api3d_charts PRIVATE
        CWAPI3D_PLUGIN_NAME=L"cw_api3d_charts"
        CWAPI3D_AUTHOR_NAME=L"michael.brunner"
        CWAPI3D_AUTHOR_EMAIL=L"michael.brunner@cadwork.ch"
)

if (TARGET cw_api3d_charts_driving)
    target_sources(cw_api3d_charts_composition PRIVATE
            StatisticsPanelWiring.cpp
            StatisticsPanelWiring.h
    )
    target_link_libraries(cw_api3d_charts_composition PUBLIC
            cw_api3d_charts_driving
    )
    target_compile_definitions(cw_api3d_charts_composition PUBLIC
            CW_API3D_HAS_DRIVING
    )
endif ()

cadwork_add_post_build_deploy(cw_api3d_charts)
