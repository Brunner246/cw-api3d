if(NOT CUSTOM_QT_PATH)
    message(STATUS "CUSTOM_QT_PATH is empty; skipping cw_api3d_driving (cadlib Qt not configured)")
    return()
endif()

list(PREPEND CMAKE_PREFIX_PATH "${CUSTOM_QT_PATH}")
find_package(Qt6 6.8 REQUIRED COMPONENTS
        Core
        Gui
        Widgets
        Qml
        Quick
        QuickControls2
        QuickWidgets
        Graphs
)

add_library(cw_api3d_driving STATIC
        statistics/StatisticsViewModel.cpp
        statistics/StatisticsViewModel.h
        statistics/BucketListModel.cpp
        statistics/BucketListModel.h
        statistics/StatisticsDockWidget.cpp
        statistics/StatisticsDockWidget.h
        statistics/HostMainWindowLocator.h
)

set_target_properties(cw_api3d_driving PROPERTIES
        AUTOMOC ON
)

set_source_files_properties(statistics/StatisticsPanel.qml PROPERTIES
        QT_RESOURCE_ALIAS StatisticsPanel.qml
)

qt_add_qml_module(cw_api3d_driving
        URI CwApi3d.Statistics
        VERSION 1.0
        QML_FILES statistics/StatisticsPanel.qml
        RESOURCE_PREFIX /qt/qml
        OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/CwApi3d/Statistics"
        NO_PLUGIN
)

target_include_directories(cw_api3d_driving PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(cw_api3d_driving
        PUBLIC
            cw_api3d_application
            cw_api3d_ports
            Qt6::Core
            Qt6::Gui
            Qt6::Widgets
            Qt6::Qml
            Qt6::Quick
            Qt6::QuickControls2
            Qt6::QuickWidgets
            Qt6::Graphs
)

cmake_path(NORMAL_PATH CUSTOM_QT_PATH OUTPUT_VARIABLE CW_NORMALIZED_QT_PATH)
target_compile_definitions(cw_api3d_driving PUBLIC
        CW_CUSTOM_QT_PATH="${CW_NORMALIZED_QT_PATH}"
)

target_compile_features(cw_api3d_driving PUBLIC cxx_std_23)
