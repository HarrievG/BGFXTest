#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "imguizmo::imguizmo" for configuration "Debug"
set_property(TARGET imguizmo::imguizmo APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(imguizmo::imguizmo PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/lib/imguizmo.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS imguizmo::imguizmo )
list(APPEND _IMPORT_CHECK_FILES_FOR_imguizmo::imguizmo "${_IMPORT_PREFIX}/debug/lib/imguizmo.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
