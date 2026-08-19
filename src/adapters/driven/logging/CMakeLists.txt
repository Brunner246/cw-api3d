add_library(cw_api3d_spdlog_adapter
        SpdLogLogger.cpp
        SpdLogLogger.h
)

target_include_directories(cw_api3d_spdlog_adapter PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(cw_api3d_spdlog_adapter
        PUBLIC cw_api3d_ports
        PRIVATE spdlog::spdlog
)

target_compile_features(cw_api3d_spdlog_adapter PUBLIC cxx_std_23)
