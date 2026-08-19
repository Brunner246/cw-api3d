# ------------------------------------------------------------------------------
# Consume cwapi3d by fetching the upstream cwapi3dcpp repository with FetchContent.
# ------------------------------------------------------------------------------

if (TARGET CwAPI3D::CwAPI3D)
    return()
endif ()

set(CWAPI3D_GIT_REPOSITORY "https://github.com/cwapi3d/cwapi3dcpp.git"
    CACHE STRING "cwapi3dcpp git repository URL (CWAPI3D_SOURCE=fetchcontent)")
set(CWAPI3D_GIT_TAG "55755e8516e995c1a6d0340e961e3cd6cf616e53"
    CACHE STRING "cwapi3dcpp git commit / tag to fetch (CWAPI3D_SOURCE=fetchcontent)")

include(FetchContent)

FetchContent_Declare(
    cwapi3dcpp
    GIT_REPOSITORY "${CWAPI3D_GIT_REPOSITORY}"
    GIT_TAG "${CWAPI3D_GIT_TAG}"
    SYSTEM
    EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(cwapi3dcpp)

if (NOT TARGET CwAPI3D::CwAPI3D)
    message(FATAL_ERROR
        "cwapi3dcpp was fetched from '${CWAPI3D_GIT_REPOSITORY}' @ "
        "'${CWAPI3D_GIT_TAG}' but it did not define the CwAPI3D::CwAPI3D target.")
endif ()

message(STATUS "cwapi3d: using FetchContent ${CWAPI3D_GIT_REPOSITORY} @ ${CWAPI3D_GIT_TAG}")
