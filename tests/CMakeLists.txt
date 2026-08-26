find_package(GTest CONFIG REQUIRED)

# Port Interface Contract Tests
add_executable(PortContractsTests
    ports/PortContractsTests.cpp
)
target_link_libraries(PortContractsTests PRIVATE
    cw_api3d_ports
    GTest::gtest
    GTest::gtest_main
)
target_include_directories(PortContractsTests PRIVATE
    ${CMAKE_SOURCE_DIR}
)
target_compile_features(PortContractsTests PRIVATE cxx_std_23)

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

include(GoogleTest)
gtest_discover_tests(PortContractsTests)
gtest_discover_tests(QueryPluginPathUseCaseTests)
gtest_discover_tests(SpdLogLoggerTests)
gtest_discover_tests(UtilityControllerAdapterTests)
gtest_discover_tests(CompositionRootTests)


