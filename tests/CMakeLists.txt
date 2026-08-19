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

# Cadwork Utility Driven Adapter Tests
add_executable(CadworkUtilityAdapterTests
    adapters/CadworkUtilityAdapterTests.cpp
)
target_link_libraries(CadworkUtilityAdapterTests PRIVATE
    cw_api3d_cadwork_adapter
    cw_api3d_ports
    CwAPI3D::CwAPI3D
    GTest::gtest
    GTest::gtest_main
)
target_include_directories(CadworkUtilityAdapterTests PRIVATE
    ${CMAKE_SOURCE_DIR}
)
target_compile_features(CadworkUtilityAdapterTests PRIVATE cxx_std_23)

include(GoogleTest)
gtest_discover_tests(PortConceptsTests)
gtest_discover_tests(QueryPluginPathUseCaseTests)
gtest_discover_tests(SpdLogLoggerTests)
gtest_discover_tests(CadworkUtilityAdapterTests)

