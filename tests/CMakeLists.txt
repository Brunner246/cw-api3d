find_package(GTest CONFIG REQUIRED)

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

include(GoogleTest)
gtest_discover_tests(PortConceptsTests)
