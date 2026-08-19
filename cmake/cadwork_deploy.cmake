# ------------------------------------------------------------------------------
# Automated Post-Build Deployment to Cadwork Userprofile
# ------------------------------------------------------------------------------

function(cadwork_add_post_build_deploy target_name)
    if (NOT WIN32)
        return()
    endif ()

    set(_cw_userprofile "")

    # 1. Explicit CMake variable override
    if (DEFINED CADWORK_USERPROFILE_DIR AND EXISTS "${CADWORK_USERPROFILE_DIR}")
        set(_cw_userprofile "${CADWORK_USERPROFILE_DIR}")
    # 2. Environment variable overrides
    elseif (DEFINED ENV{CADWORK_USP} AND EXISTS "$ENV{CADWORK_USP}")
        set(_cw_userprofile "$ENV{CADWORK_USP}")
    elseif (DEFINED ENV{CISTART_USP} AND EXISTS "$ENV{CISTART_USP}")
        set(_cw_userprofile "$ENV{CISTART_USP}")
    # 3. Windows Registry query (CADWORK_USP or CISTART_USP)
    else ()
        cmake_host_system_information(RESULT _reg_cadwork_usp
            QUERY WINDOWS_REGISTRY "HKCU/Software/cadwork Informatik/ENV"
            VALUE "CADWORK_USP"
        )
        if (_reg_cadwork_usp AND EXISTS "${_reg_cadwork_usp}")
            set(_cw_userprofile "${_reg_cadwork_usp}")
        else ()
            cmake_host_system_information(RESULT _reg_cistart_usp
                QUERY WINDOWS_REGISTRY "HKCU/Software/cadwork Informatik/ENV"
                VALUE "CISTART_USP"
            )
            if (_reg_cistart_usp AND EXISTS "${_reg_cistart_usp}")
                set(_cw_userprofile "${_reg_cistart_usp}")
            endif ()
        endif ()
    endif ()

    if (NOT _cw_userprofile)
        message(STATUS "cadwork: userprofile path not found in registry or environment; skipping post-build deployment for '${target_name}'.")
        return()
    endif ()

    cmake_path(NORMAL_PATH _cw_userprofile)
    set(_deploy_dir "${_cw_userprofile}/3d/API.x64/${target_name}")

    add_custom_command(
        TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_deploy_dir}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:${target_name}>"
                "${_deploy_dir}/$<TARGET_FILE_NAME:${target_name}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_RUNTIME_DLLS:${target_name}>"
                "${_deploy_dir}"
        COMMAND_EXPAND_LISTS
        COMMENT "Deploying ${target_name} and runtime DLLs to ${_deploy_dir}"
        VERBATIM
    )

    message(STATUS "cadwork: registered post-build deploy for '${target_name}' -> '${_deploy_dir}'")
endfunction()
