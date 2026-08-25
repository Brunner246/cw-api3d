find_package(GTest CONFIG REQUIRED)

# Port Concepts & Interfaces Tests
add_executable(PortConceptsTests
    ports/PortConceptsTests.cpp
)
target_link_libraries(PortConceptsTests PRIVATE
    cw_api3d_ports
    GTest::gtest
    GTest::gtest_main
)
target_include_directories(PortConceptsTests PRIVATE
    ${CMAKE_SOURCE_DIR}
)
target_compile_features(PortConceptsTests PRIVATE cxx_std_23)

# Application Use Case Tests
add_executable(QueryPluginPathUseCaseTests
    application/QueryPluginPathUseCaseTests.cpp
)
target_link_libraries(QueryPluginPathUseCaseTests PRIVATE
    cw_api3d_application
    cw_api3d_ports
    GTest::gtest
    GTest::gtest_main
)
target_include_directories(QueryPluginPathUseCaseTests PRIVATE
    ${CMAKE_SOURCE_DIR}
)
target_compile_features(QueryPluginPathUseCaseTests PRIVATE cxx_std_23)

# SpdLog Driven Logger Adapter Tests
add_executable(SpdLogLoggerTests
    adapters/SpdLogLoggerTests.cpp
)
target_link_libraries(SpdLogLoggerTests PRIVATE
    cw_api3d_spdlog_adapter
    cw_api3d_ports
    spdlog::spdlog
    GTest::gtest
    GTest::gtest_main
)
target_include_directories(SpdLogLoggerTests PRIVATE
    ${CMAKE_SOURCE_DIR}
)
target_compile_features(SpdLogLoggerTests PRIVATE cxx_std_23)

# Utility Controller Driven Adapter Tests (SDK-free: adapter is constrained on a concept)
add_executable(UtilityControllerAdapterTests
    adapters/UtilityControllerAdapterTests.cpp
)
target_link_libraries(UtilityControllerAdapterTests PRIVATE
    cw_api3d_ports
    GTest::gtest
    GTest::gtest_main
)
target_include_directories(UtilityControllerAdapterTests PRIVATE
    ${CMAKE_SOURCE_DIR}
)
target_compile_features(UtilityControllerAdapterTests PRIVATE cxx_std_23)

# Composition Root Tests
add_executable(CompositionRootTests
    composition/CompositionRootTests.cpp
)
target_link_libraries(CompositionRootTests PRIVATE
    cw_api3d_composition
    cw_api3d_ports
    cw_api3d_application
    cw_api3d_spdlog_adapter
    GTest::gtest
    GTest::gtest_main
)
target_include_directories(CompositionRootTests PRIVATE
    ${CMAKE_SOURCE_DIR}
)
target_compile_features(CompositionRootTests PRIVATE cxx_std_23)

add_executable(HostAbsentQtRuntimeVisibilityTests
    composition/HostAbsentQtRuntimeVisibilityTests.cpp
)
target_link_libraries(HostAbsentQtRuntimeVisibilityTests PRIVATE
    cw_api3d_composition
    cw_api3d_ports
    GTest::gtest
    GTest::gtest_main
)
target_include_directories(HostAbsentQtRuntimeVisibilityTests PRIVATE
    ${CMAKE_SOURCE_DIR}
)
target_compile_features(HostAbsentQtRuntimeVisibilityTests PRIVATE cxx_std_23)

# Producer verification fixtures (header-only; no Qt, no production ports)
add_executable(ElementSnapshotFixturesTests
    fixtures/ElementSnapshotFixturesTests.cpp
)
target_link_libraries(ElementSnapshotFixturesTests PRIVATE
    GTest::gtest
    GTest::gtest_main
)
target_include_directories(ElementSnapshotFixturesTests PRIVATE
    ${CMAKE_SOURCE_DIR}
)
target_compile_features(ElementSnapshotFixturesTests PRIVATE cxx_std_23)

add_executable(ContrastTokenPairsTests
    fixtures/ContrastTokenPairsTests.cpp
)
target_link_libraries(ContrastTokenPairsTests PRIVATE
    GTest::gtest
    GTest::gtest_main
)
target_include_directories(ContrastTokenPairsTests PRIVATE
    ${CMAKE_SOURCE_DIR}
)
target_compile_features(ContrastTokenPairsTests PRIVATE cxx_std_23)

add_executable(ElementStatisticsAggregatorTests
    application/ElementStatisticsAggregatorTests.cpp
)
target_link_libraries(ElementStatisticsAggregatorTests PRIVATE
    cw_api3d_application
    cw_api3d_ports
    GTest::gtest
    GTest::gtest_main
)
target_include_directories(ElementStatisticsAggregatorTests PRIVATE
    ${CMAKE_SOURCE_DIR}
)
target_compile_features(ElementStatisticsAggregatorTests PRIVATE cxx_std_23)

add_executable(FetchElementSnapshotUseCaseTests
    application/FetchElementSnapshotUseCaseTests.cpp
)
target_link_libraries(FetchElementSnapshotUseCaseTests PRIVATE
    cw_api3d_application
    cw_api3d_ports
    GTest::gtest
    GTest::gtest_main
)
target_include_directories(FetchElementSnapshotUseCaseTests PRIVATE
    ${CMAKE_SOURCE_DIR}
)
target_compile_features(FetchElementSnapshotUseCaseTests PRIVATE cxx_std_23)

add_executable(ActivateElementsUseCaseTests
    application/ActivateElementsUseCaseTests.cpp
)
target_link_libraries(ActivateElementsUseCaseTests PRIVATE
    cw_api3d_application
    cw_api3d_ports
    GTest::gtest
    GTest::gtest_main
)
target_include_directories(ActivateElementsUseCaseTests PRIVATE
    ${CMAKE_SOURCE_DIR}
)
target_compile_features(ActivateElementsUseCaseTests PRIVATE cxx_std_23)

add_executable(ElementCatalogAdapterTests
    adapters/ElementCatalogAdapterTests.cpp
)
target_link_libraries(ElementCatalogAdapterTests PRIVATE
    cw_api3d_ports
    GTest::gtest
    GTest::gtest_main
)
target_include_directories(ElementCatalogAdapterTests PRIVATE
    ${CMAKE_SOURCE_DIR}
)
target_compile_features(ElementCatalogAdapterTests PRIVATE cxx_std_23)

add_executable(ElementActivationAdapterTests
    adapters/ElementActivationAdapterTests.cpp
)
target_link_libraries(ElementActivationAdapterTests PRIVATE
    cw_api3d_ports
    GTest::gtest
    GTest::gtest_main
)
target_include_directories(ElementActivationAdapterTests PRIVATE
    ${CMAKE_SOURCE_DIR}
)
target_compile_features(ElementActivationAdapterTests PRIVATE cxx_std_23)

include(GoogleTest)
gtest_discover_tests(PortConceptsTests)
gtest_discover_tests(QueryPluginPathUseCaseTests)
gtest_discover_tests(SpdLogLoggerTests)
gtest_discover_tests(UtilityControllerAdapterTests)
gtest_discover_tests(CompositionRootTests
    DISCOVERY_MODE PRE_TEST
)
gtest_discover_tests(HostAbsentQtRuntimeVisibilityTests)
gtest_discover_tests(ElementSnapshotFixturesTests)
gtest_discover_tests(ContrastTokenPairsTests)
gtest_discover_tests(ElementStatisticsAggregatorTests)
gtest_discover_tests(FetchElementSnapshotUseCaseTests)
gtest_discover_tests(ActivateElementsUseCaseTests)
gtest_discover_tests(ElementCatalogAdapterTests)
gtest_discover_tests(ElementActivationAdapterTests)

add_test(
    NAME CopyNonQtRuntimeDllsTests.CopiesGraphsRuntimeClosureAndSkipsHostQtCore
    COMMAND ${CMAKE_COMMAND}
        "-DCOPY_SCRIPT=${CMAKE_SOURCE_DIR}/cmake/copy_non_qt_runtime_dlls.cmake"
        "-DTEST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/CopyNonQtRuntimeDllsTests"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyNonQtRuntimeDllsTests.cmake"
)

if(CUSTOM_QT_PATH)
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

    if(TARGET cw_api3d_driving)
        add_executable(StatisticsViewModelTests
            adapters/StatisticsViewModelTests.cpp
        )
        target_link_libraries(StatisticsViewModelTests PRIVATE
            cw_api3d_driving
            cw_api3d_application
            cw_api3d_ports
            Qt6::Core
            GTest::gtest
        )
        target_include_directories(StatisticsViewModelTests PRIVATE
            ${CMAKE_SOURCE_DIR}
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
            cw_api3d_driving
            Qt6::Core
            Qt6::Gui
            GTest::gtest
        )
        target_include_directories(CategoricalPaletteTests PRIVATE
            ${CMAKE_SOURCE_DIR}
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
            cw_api3d_driving
            cw_api3d_application
            cw_api3d_ports
            Qt6::Core
            Qt6::Gui
            Qt6::Widgets
            Qt6::Quick
            Qt6::QuickWidgets
            GTest::gtest
        )
        target_include_directories(StatisticsDockWidgetTests PRIVATE
            ${CMAKE_SOURCE_DIR}
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
            cw_api3d_driving
            cw_api3d_application
            cw_api3d_ports
            Qt6::Core
            Qt6::Gui
            Qt6::Quick
            Qt6::QuickTest
            Qt6::Qml
        )
        target_include_directories(tst_StatisticsPanel PRIVATE
            ${CMAKE_SOURCE_DIR}
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
    endif()
else()
    message(STATUS "CUSTOM_QT_PATH is empty; skipping tst_HarnessSmoke (cadlib Qt not configured)")
endif()


