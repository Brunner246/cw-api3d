include(GoogleTest)

function(cw_kit_gtest name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE
            cw_api3d_ports
            GTest::gtest
            GTest::gtest_main
    )
    target_include_directories(${name} PRIVATE
            ${CW_API3D_KIT_ROOT}
    )
    target_compile_features(${name} PRIVATE cxx_std_23)
    gtest_discover_tests(${name})
endfunction()

cw_kit_gtest(PortConceptsTests
        ports/PortConceptsTests.cpp
)

cw_kit_gtest(QueryPluginPathUseCaseTests
        application/QueryPluginPathUseCaseTests.cpp
)
target_link_libraries(QueryPluginPathUseCaseTests PRIVATE
        cw_api3d_application
)

cw_kit_gtest(SpdLogLoggerTests
        adapters/SpdLogLoggerTests.cpp
)
target_link_libraries(SpdLogLoggerTests PRIVATE
        cw_api3d_spdlog_adapter
        spdlog::spdlog
)

cw_kit_gtest(UtilityControllerAdapterTests
        adapters/UtilityControllerAdapterTests.cpp
)

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
        ${CW_API3D_KIT_ROOT}
)
target_compile_features(CompositionRootTests PRIVATE cxx_std_23)
gtest_discover_tests(CompositionRootTests
        DISCOVERY_MODE PRE_TEST
)
