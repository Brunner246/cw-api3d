include(GoogleTest)

function(cw_charts_gtest name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE
            cw_api3d_charts_application
            cw_api3d_charts_ports
            cw_api3d_ports
            GTest::gtest
            GTest::gtest_main
    )
    target_include_directories(${name} PRIVATE
            ${CW_API3D_CHARTS_ROOT}
            ${CW_API3D_KIT_ROOT}
    )
    target_compile_features(${name} PRIVATE cxx_std_23)
    gtest_discover_tests(${name})
endfunction()

cw_charts_gtest(ChartsPortConceptsTests
        ports/PortConceptsTests.cpp
)

cw_charts_gtest(ElementSnapshotFixturesTests
        fixtures/ElementSnapshotFixturesTests.cpp
)

cw_charts_gtest(ContrastTokenPairsTests
        fixtures/ContrastTokenPairsTests.cpp
)

cw_charts_gtest(ElementStatisticsAggregatorTests
        application/ElementStatisticsAggregatorTests.cpp
)

cw_charts_gtest(FetchElementSnapshotUseCaseTests
        application/FetchElementSnapshotUseCaseTests.cpp
)

cw_charts_gtest(ActivateElementsUseCaseTests
        application/ActivateElementsUseCaseTests.cpp
)

cw_charts_gtest(ElementCatalogAdapterTests
        adapters/ElementCatalogAdapterTests.cpp
)

cw_charts_gtest(ElementActivationAdapterTests
        adapters/ElementActivationAdapterTests.cpp
)

add_executable(HostAbsentQtRuntimeVisibilityTests
        composition/HostAbsentQtRuntimeVisibilityTests.cpp
)
target_link_libraries(HostAbsentQtRuntimeVisibilityTests PRIVATE
        cw_api3d_charts_composition
        cw_api3d_ports
        GTest::gtest
        GTest::gtest_main
)
target_include_directories(HostAbsentQtRuntimeVisibilityTests PRIVATE
        ${CW_API3D_CHARTS_ROOT}
        ${CW_API3D_KIT_ROOT}
)
target_compile_features(HostAbsentQtRuntimeVisibilityTests PRIVATE cxx_std_23)
gtest_discover_tests(HostAbsentQtRuntimeVisibilityTests)

add_executable(PluginUiSessionTests
        composition/PluginUiSessionTests.cpp
)
target_link_libraries(PluginUiSessionTests PRIVATE
        cw_api3d_charts_composition
        cw_api3d_ports
        GTest::gtest
        GTest::gtest_main
)
target_include_directories(PluginUiSessionTests PRIVATE
        ${CW_API3D_CHARTS_ROOT}
        ${CW_API3D_KIT_ROOT}
)
target_compile_features(PluginUiSessionTests PRIVATE cxx_std_23)
gtest_discover_tests(PluginUiSessionTests
        DISCOVERY_MODE PRE_TEST
)

add_test(
        NAME CopyNonQtRuntimeDllsTests.CopiesGraphsRuntimeClosureAndSkipsHostQtCore
        COMMAND ${CMAKE_COMMAND}
        "-DCOPY_SCRIPT=${CMAKE_SOURCE_DIR}/cmake/copy_non_qt_runtime_dlls.cmake"
        "-DTEST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/CopyNonQtRuntimeDllsTests"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyNonQtRuntimeDllsTests.cmake"
)

if (CUSTOM_QT_PATH)
    list(PREPEND CMAKE_PREFIX_PATH "${CUSTOM_QT_PATH}")
    find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui QuickTest Widgets QuickWidgets Qml Quick)
    find_program(QMLTESTRUNNER_EXECUTABLE
            NAMES qmltestrunner qmltestrunner.exe
            HINTS "${CUSTOM_QT_PATH}/bin"
            REQUIRED
    )
    set(CW_API3D_QT_TEST_ENV
            "PATH=path_list_prepend:${CUSTOM_QT_PATH}/bin;QT_QPA_PLATFORM=set:offscreen;QT_PLUGIN_PATH=set:${CUSTOM_QT_PATH}/plugins;QML2_IMPORT_PATH=set:${CUSTOM_QT_PATH}/qml;QML_IMPORT_PATH=set:${CUSTOM_QT_PATH}/qml"
    )
    add_test(
            NAME tst_HarnessSmoke
            COMMAND "${QMLTESTRUNNER_EXECUTABLE}"
            -input "${CMAKE_CURRENT_SOURCE_DIR}/qml/tst_HarnessSmoke.qml"
    )
    set_tests_properties(tst_HarnessSmoke PROPERTIES
            ENVIRONMENT_MODIFICATION
            "${CW_API3D_QT_TEST_ENV}"
    )

    if (TARGET cw_api3d_charts_driving)
        add_executable(StatisticsViewModelTests
                adapters/StatisticsViewModelTests.cpp
        )
        target_link_libraries(StatisticsViewModelTests PRIVATE
                cw_api3d_charts_driving
                cw_api3d_charts_application
                cw_api3d_charts_ports
                cw_api3d_ports
                Qt6::Core
                GTest::gtest
        )
        target_include_directories(StatisticsViewModelTests PRIVATE
                ${CW_API3D_CHARTS_ROOT}
                ${CW_API3D_KIT_ROOT}
        )
        target_compile_features(StatisticsViewModelTests PRIVATE cxx_std_23)
        gtest_discover_tests(StatisticsViewModelTests
                WORKING_DIRECTORY "${CUSTOM_QT_PATH}/bin"
                DISCOVERY_MODE PRE_TEST
                PROPERTIES
                ENVIRONMENT_MODIFICATION
                "${CW_API3D_QT_TEST_ENV}"
        )

        add_executable(CategoricalPaletteTests
                adapters/CategoricalPaletteTests.cpp
        )
        target_link_libraries(CategoricalPaletteTests PRIVATE
                cw_api3d_charts_driving
                Qt6::Core
                Qt6::Gui
                GTest::gtest
        )
        target_include_directories(CategoricalPaletteTests PRIVATE
                ${CW_API3D_CHARTS_ROOT}
                ${CW_API3D_KIT_ROOT}
        )
        target_compile_features(CategoricalPaletteTests PRIVATE cxx_std_23)
        gtest_discover_tests(CategoricalPaletteTests
                WORKING_DIRECTORY "${CUSTOM_QT_PATH}/bin"
                DISCOVERY_MODE PRE_TEST
                PROPERTIES
                ENVIRONMENT_MODIFICATION
                "${CW_API3D_QT_TEST_ENV}"
        )

        add_executable(StatisticsDockWidgetTests
                adapters/StatisticsDockWidgetTests.cpp
        )
        target_link_libraries(StatisticsDockWidgetTests PRIVATE
                cw_api3d_charts_driving
                cw_api3d_charts_application
                cw_api3d_charts_ports
                cw_api3d_ports
                Qt6::Core
                Qt6::Gui
                Qt6::Widgets
                Qt6::Quick
                Qt6::QuickWidgets
                GTest::gtest
        )
        target_include_directories(StatisticsDockWidgetTests PRIVATE
                ${CW_API3D_CHARTS_ROOT}
                ${CW_API3D_KIT_ROOT}
        )
        target_compile_features(StatisticsDockWidgetTests PRIVATE cxx_std_23)
        gtest_discover_tests(StatisticsDockWidgetTests
                WORKING_DIRECTORY "${CUSTOM_QT_PATH}/bin"
                DISCOVERY_MODE PRE_TEST
                PROPERTIES
                ENVIRONMENT_MODIFICATION
                "${CW_API3D_QT_TEST_ENV}"
        )

        add_executable(tst_StatisticsPanel
                qml/tst_StatisticsPanelSetup.cpp
        )
        set_target_properties(tst_StatisticsPanel PROPERTIES
                AUTOMOC ON
                WIN32_EXECUTABLE FALSE
        )
        target_link_libraries(tst_StatisticsPanel PRIVATE
                cw_api3d_charts_driving
                cw_api3d_charts_application
                cw_api3d_charts_ports
                cw_api3d_ports
                Qt6::Core
                Qt6::Gui
                Qt6::Quick
                Qt6::QuickTest
                Qt6::Qml
        )
        target_include_directories(tst_StatisticsPanel PRIVATE
                ${CW_API3D_CHARTS_ROOT}
                ${CW_API3D_KIT_ROOT}
        )
        target_compile_features(tst_StatisticsPanel PRIVATE cxx_std_23)
        target_compile_definitions(tst_StatisticsPanel PRIVATE
                QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/qml"
        )
        add_test(
                NAME tst_StatisticsPanel
                COMMAND tst_StatisticsPanel -input "${CMAKE_CURRENT_SOURCE_DIR}/qml" -o -,txt
                WORKING_DIRECTORY "${CUSTOM_QT_PATH}/bin"
        )
        set_tests_properties(tst_StatisticsPanel PROPERTIES
                ENVIRONMENT_MODIFICATION
                "${CW_API3D_QT_TEST_ENV}"
        )
    endif ()
else ()
    message(STATUS "CUSTOM_QT_PATH is empty; skipping tst_HarnessSmoke (cadlib Qt not configured)")
endif ()
