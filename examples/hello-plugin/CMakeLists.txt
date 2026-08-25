add_library(cw_api3d_hello SHARED
        src/composition/PluginEntry.cpp
)

target_link_libraries(cw_api3d_hello PRIVATE
        cw_api3d_composition
)

target_include_directories(cw_api3d_hello PRIVATE
        ${CW_API3D_KIT_ROOT}
)

target_compile_features(cw_api3d_hello PRIVATE cxx_std_23)

target_compile_definitions(cw_api3d_hello PRIVATE
        CWAPI3D_PLUGIN_NAME=L"cw_api3d_hello"
        CWAPI3D_AUTHOR_NAME=L"michael.brunner"
        CWAPI3D_AUTHOR_EMAIL=L"michael.brunner@cadwork.ch"
)

cadwork_add_post_build_deploy(cw_api3d_hello)
