# Hermetic copier contract: Graphs closure + non-Qt copied, host Qt6Core skipped.
# Empty stub files, not authentic PE. One allowlist name is omitted from FILES
# so QT_BIN_DIR backfill is required (QuickShapes TARGET_RUNTIME_DLLS hole).

if(NOT DEFINED COPY_SCRIPT OR COPY_SCRIPT STREQUAL "")
  message(FATAL_ERROR "COPY_SCRIPT is required")
endif()
if(NOT DEFINED TEST_ROOT OR TEST_ROOT STREQUAL "")
  message(FATAL_ERROR "TEST_ROOT is required")
endif()

file(REMOVE_RECURSE "${TEST_ROOT}")
set(filesDir "${TEST_ROOT}/files")
set(qtBinDir "${TEST_ROOT}/qtbin")
set(destDir "${TEST_ROOT}/dest")
file(MAKE_DIRECTORY "${filesDir}")
file(MAKE_DIRECTORY "${qtBinDir}")
file(MAKE_DIRECTORY "${destDir}")

set(allowlistNames
  Qt6Graphs.dll
  Qt6QuickShapes.dll
  Qt6Quick3D.dll
  Qt6Quick3DRuntimeRender.dll
  Qt6Quick3DUtils.dll
  Qt6ShaderTools.dll
)

foreach(name IN ITEMS
    Qt6Core.dll
    Qt6Graphs.dll
    Qt6Quick3D.dll
    Qt6Quick3DRuntimeRender.dll
    Qt6Quick3DUtils.dll
    Qt6ShaderTools.dll
    fmt.dll)
  file(WRITE "${filesDir}/${name}" "")
endforeach()
file(WRITE "${qtBinDir}/Qt6QuickShapes.dll" "")

set(DEST_DIR "${destDir}")
set(QT_BIN_DIR "${qtBinDir}")
set(FILES
  "${filesDir}/Qt6Core.dll"
  "${filesDir}/Qt6Graphs.dll"
  "${filesDir}/Qt6Quick3D.dll"
  "${filesDir}/Qt6Quick3DRuntimeRender.dll"
  "${filesDir}/Qt6Quick3DUtils.dll"
  "${filesDir}/Qt6ShaderTools.dll"
  "${filesDir}/fmt.dll"
)

include("${COPY_SCRIPT}")

foreach(name IN LISTS allowlistNames)
  if(NOT EXISTS "${destDir}/${name}")
    message(FATAL_ERROR "all Qt[0-9]* skipped / Graphs closure absent: expected ${name} in dest")
  endif()
endforeach()
if(NOT EXISTS "${destDir}/fmt.dll")
  message(FATAL_ERROR "expected non-Qt runtime fmt.dll in dest")
endif()
if(EXISTS "${destDir}/Qt6Core.dll")
  message(FATAL_ERROR "host Qt6Core.dll must not be copied into dest")
endif()
