# ------------------------------------------------------------------------------
# cwapi3d acquisition dispatcher.
# ------------------------------------------------------------------------------

if (TARGET CwAPI3D::CwAPI3D)
    return()
endif ()

set(CWAPI3D_SOURCE "fetchcontent" CACHE STRING
    "How to obtain cwapi3d: 'fetchcontent' (GitHub, default) or 'cadlib' (local cadlib source tree)")
set_property(CACHE CWAPI3D_SOURCE PROPERTY STRINGS fetchcontent cadlib)

if (CWAPI3D_SOURCE STREQUAL "fetchcontent")
    include("${CMAKE_CURRENT_LIST_DIR}/CwAPI3DFromFetchContent.cmake")
elseif (CWAPI3D_SOURCE STREQUAL "cadlib")
    include("${CMAKE_CURRENT_LIST_DIR}/CwAPI3DFromCadlib.cmake")
else ()
    message(FATAL_ERROR
        "Invalid CWAPI3D_SOURCE='${CWAPI3D_SOURCE}' — expected 'fetchcontent' or 'cadlib'.")
endif ()
