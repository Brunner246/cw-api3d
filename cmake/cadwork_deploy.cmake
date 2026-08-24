# ------------------------------------------------------------------------------
# Automated Post-Build Deployment to Cadwork Userprofile
# ------------------------------------------------------------------------------

# Keep in sync with tests/e2e/cadwork_paths.py and build-scripts/new-local-profile.ps1.
set(CADWORK_USERPROFILE_MARKER_NAME ".cw-userprofile")

# Reads the checkout-local userprofile marker written by build-scripts/new-local-profile.ps1.
# Format: first line that is neither blank nor a '#' comment is the profile path.
function(_cadwork_read_userprofile_marker out_var)
    set(${out_var} "" PARENT_SCOPE)

    set(_marker "${CMAKE_SOURCE_DIR}/${CADWORK_USERPROFILE_MARKER_NAME}")
    if (NOT EXISTS "${_marker}")
        return()
    endif ()

    # Re-configure when the marker changes, so a repointed worktree is not stuck on a stale path.
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_marker}")

    file(STRINGS "${_marker}" _lines)
    foreach (_line IN LISTS _lines)
        string(STRIP "${_line}" _line)
        if (_line STREQUAL "" OR _line MATCHES "^#")
            continue()
        endif ()
        set(${out_var} "${_line}" PARENT_SCOPE)
        return()
    endforeach ()
endfunction()

function(cadwork_add_post_build_deploy target_name)
    if (NOT WIN32)
        return()
    endif ()

    set(_cw_userprofile "")

    # 1. Checkout-local marker (per-worktree profile) -- deliberately ahead of the CMake variable,
    #    which a worktree inherits verbatim from the CMakeUserPresets.json it was copied from.
    _cadwork_read_userprofile_marker(_cw_marker_profile)
    if (_cw_marker_profile AND EXISTS "${_cw_marker_profile}")
        set(_cw_userprofile "${_cw_marker_profile}")
    # 2. Explicit CMake variable override
    elseif (DEFINED CADWORK_USERPROFILE_DIR AND EXISTS "${CADWORK_USERPROFILE_DIR}")
        set(_cw_userprofile "${CADWORK_USERPROFILE_DIR}")
    # 3. Environment variable overrides
    elseif (DEFINED ENV{CADWORK_USP} AND EXISTS "$ENV{CADWORK_USP}")
        set(_cw_userprofile "$ENV{CADWORK_USP}")
    elseif (DEFINED ENV{CISTART_USP} AND EXISTS "$ENV{CISTART_USP}")
        set(_cw_userprofile "$ENV{CISTART_USP}")
    # 4. Windows Registry query (CADWORK_USP or CISTART_USP)
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
        message(STATUS "cadwork: userprofile path not found in ${CADWORK_USERPROFILE_MARKER_NAME}, environment or registry; skipping post-build deployment for '${target_name}'.")
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
        COMMAND ${CMAKE_COMMAND}
                "-DDEST_DIR=${_deploy_dir}"
                "-DFILES=$<TARGET_RUNTIME_DLLS:${target_name}>"
                -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/copy_non_qt_runtime_dlls.cmake"
        COMMAND_EXPAND_LISTS
        COMMENT "Deploying ${target_name} and non-Qt runtime DLLs to ${_deploy_dir}"
        VERBATIM
    )

    message(STATUS "cadwork: registered post-build deploy for '${target_name}' -> '${_deploy_dir}'")
endfunction()
