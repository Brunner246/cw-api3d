# ------------------------------------------------------------------------------
# Consume cwapi3d directly from the cadlib source tree (v_33.0) via compat junction.
# ------------------------------------------------------------------------------

if (TARGET CwAPI3D::CwAPI3D)
    return()
endif ()

if (NOT DEFINED CWAPI3D_CADLIB_INCLUDE_DIR)
    message(FATAL_ERROR
        "CWAPI3D_CADLIB_INCLUDE_DIR is not set. It must point at the cadlib "
        "CwAPI3D/include directory (set it in CMakeUserPresets.json, e.g. "
        "\"$env{CADLIB_DIR}/v_33.0/3d/CwAPI3D/include\", or pass -D on the CI "
        "configure line).")
endif ()

set(_cw_compat_root "${CMAKE_BINARY_DIR}/cwapi3d-compat")
set(_cw_compat_link "${_cw_compat_root}/cwapi3d")
set(_cw_stamp "${_cw_compat_root}/.cwapi3d-target.txt")

set(_cw_need_link TRUE)
if (EXISTS "${_cw_compat_link}")
    set(_cw_prev "")
    if (EXISTS "${_cw_stamp}")
        file(READ "${_cw_stamp}" _cw_prev)
        string(STRIP "${_cw_prev}" _cw_prev)
    endif ()
    if (_cw_prev STREQUAL "${CWAPI3D_CADLIB_INCLUDE_DIR}")
        set(_cw_need_link FALSE)
    else ()
        message(STATUS
            "cwapi3d: compat junction targets a stale path ('${_cw_prev}'); "
            "recreating for '${CWAPI3D_CADLIB_INCLUDE_DIR}'")
        file(TO_NATIVE_PATH "${_cw_compat_link}" _cw_link_native)
        execute_process(COMMAND cmd /c rmdir "${_cw_link_native}"
            RESULT_VARIABLE _cw_rmdir_rc)
        if (NOT _cw_rmdir_rc EQUAL 0)
            message(FATAL_ERROR "Failed to remove stale cwapi3d junction '${_cw_link_native}'")
        endif ()
    endif ()
endif ()

if (_cw_need_link)
    file(MAKE_DIRECTORY "${_cw_compat_root}")
    file(TO_NATIVE_PATH "${_cw_compat_link}" _cw_link_native)
    file(TO_NATIVE_PATH "${CWAPI3D_CADLIB_INCLUDE_DIR}" _cw_target_native)
    execute_process(
        COMMAND cmd /c mklink /J "${_cw_link_native}" "${_cw_target_native}"
        RESULT_VARIABLE _cw_mklink_rc
        OUTPUT_VARIABLE _cw_mklink_out
        ERROR_VARIABLE _cw_mklink_err
    )
    if (NOT _cw_mklink_rc EQUAL 0 OR NOT EXISTS "${_cw_compat_link}")
        message(FATAL_ERROR
            "Failed to junction the cwapi3d compat dir\n"
            "  '${_cw_link_native}' -> '${_cw_target_native}'\n"
            "  ${_cw_mklink_out}${_cw_mklink_err}")
    endif ()
    file(WRITE "${_cw_stamp}" "${CWAPI3D_CADLIB_INCLUDE_DIR}")
    message(STATUS "cwapi3d: junctioned ${_cw_compat_link} -> ${CWAPI3D_CADLIB_INCLUDE_DIR}")
endif ()

add_library(CwAPI3D::CwAPI3D INTERFACE IMPORTED GLOBAL)
set_target_properties(CwAPI3D::CwAPI3D PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${_cw_compat_root}"
)
message(STATUS "cwapi3d: using cadlib 33.0 headers from ${CWAPI3D_CADLIB_INCLUDE_DIR}")
