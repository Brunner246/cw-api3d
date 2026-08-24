# Copies runtime DLLs except Qt, so the plugin reuses cadwork's bundled Qt.
if(NOT DEFINED DEST_DIR OR DEST_DIR STREQUAL "")
  return()
endif()

if(NOT DEFINED FILES)
  return()
endif()

foreach(dll IN LISTS FILES)
  if(dll STREQUAL "")
    continue()
  endif()
  get_filename_component(name "${dll}" NAME)
  if(name MATCHES "^Qt[0-9]")
    continue()
  endif()
  file(COPY "${dll}" DESTINATION "${DEST_DIR}")
endforeach()
