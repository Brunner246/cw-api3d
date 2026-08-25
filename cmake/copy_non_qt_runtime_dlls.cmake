# Copies runtime DLLs except host Qt, plus the host-absent Graphs closure.
# Cadwork ships Core/Gui/Qml/Quick; it omits Graphs and the Quick3D/ShaderTools
# load-time imports of Qt6Graphs.dll. TARGET_RUNTIME_DLLS may omit those
# imported dependents, so missing allowlist names are backfilled from QT_BIN_DIR.
if(NOT DEFINED DEST_DIR OR DEST_DIR STREQUAL "")
  return()
endif()

if(NOT DEFINED FILES)
  return()
endif()

set(_hostAbsentQtRuntime
  Qt6Graphs.dll
  Qt6QuickShapes.dll
  Qt6Quick3D.dll
  Qt6Quick3DRuntimeRender.dll
  Qt6Quick3DUtils.dll
  Qt6ShaderTools.dll
)

foreach(dll IN LISTS FILES)
  if(dll STREQUAL "")
    continue()
  endif()
  get_filename_component(name "${dll}" NAME)
  if(name MATCHES "^Qt[0-9]")
    list(FIND _hostAbsentQtRuntime "${name}" _allowIndex)
    if(_allowIndex EQUAL -1)
      continue()
    endif()
  endif()
  file(COPY "${dll}" DESTINATION "${DEST_DIR}")
endforeach()

if(NOT DEFINED QT_BIN_DIR OR QT_BIN_DIR STREQUAL "")
  return()
endif()

foreach(name IN LISTS _hostAbsentQtRuntime)
  if(EXISTS "${DEST_DIR}/${name}")
    continue()
  endif()
  set(_source "${QT_BIN_DIR}/${name}")
  if(EXISTS "${_source}")
    file(COPY "${_source}" DESTINATION "${DEST_DIR}")
  else()
    message(WARNING "host-absent Qt runtime ${name} missing from QT_BIN_DIR=${QT_BIN_DIR}")
  endif()
endforeach()
