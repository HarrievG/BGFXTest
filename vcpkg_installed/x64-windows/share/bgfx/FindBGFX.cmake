# Distributed under the OSI-approved BSD 3-Clause License.
#
#.rst:
# FindBGFX
# --------
#
# Find the BGFX libraries
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# The following variables will be defined:
#
#  ``BGFX_FOUND``
#    True if BGFX found on the local system
#
#  ``BGFX_INCLUDE_DIRS``
#    Location of BGFX header files
#
#  ``BGFX_LIBRARY_DIRS``
#    Location of BGFX libraries
#
#  ``BGFX_LIBRARIES``
#    List of the BGFX libraries found
#
#

include(FindPackageHandleStandardArgs)
include(SelectLibraryConfigurations)
include(CMakeFindDependencyMacro)

if(NOT BGFX_FOUND)

# Compute the installation path relative to this file.
get_filename_component(SEARCH_PATH "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(SEARCH_PATH "${SEARCH_PATH}" PATH)
get_filename_component(SEARCH_PATH "${SEARCH_PATH}" PATH)
if(SEARCH_PATH STREQUAL "/")
  set(SEARCH_PATH "")
endif()

set(BGFX_VERSION "")

function(append_dependencies out)
    cmake_parse_arguments(PARSE_ARGV 1 "arg" "DEBUG" "NAMES" "")
    if(${arg_DEBUG})
        set(config DEBUG)
        set(path "${CURRENT_INSTALLED_DIR}/debug/lib/")
    else()
        set(config RELEASE)
        set(path "${CURRENT_INSTALLED_DIR}/lib/")
    endif()
    foreach(lib_name ${arg_NAMES})
        if("${lib_name}" STREQUAL "-pthread")
            list(APPEND ${out} "-pthread")
        elseif("${lib_name}" STREQUAL "-pthreads")
            list(APPEND ${out} "-pthreads")
        elseif("${lib_name}" STREQUAL "gcc")
            list(APPEND ${out} "-lgcc")
        elseif("${lib_name}" STREQUAL "gcc_s")
            list(APPEND ${out} "-lgcc_s")
        elseif("${lib_name}" STREQUAL "stdc++")
            list(APPEND ${out} "-lstdc++")
        else()
            # first look in ${path} specifically to ensure we find the right release/debug variant
            find_library(BGFX_DEPENDENCY_${lib_name}_${config} NAMES "${lib_name}" PATHS "${path}" NO_DEFAULT_PATH)
            # if not found there, must be a system dependency, so look elsewhere
            find_library(BGFX_DEPENDENCY_${lib_name}_${config} NAMES "${lib_name}" REQUIRED)
            list(APPEND ${out} "${BGFX_DEPENDENCY_${lib_name}_${config}}")
        endif()
    endforeach()
    set("${out}" "${${out}}" PARENT_SCOPE)
endfunction()

macro(BGFX_FIND libname shortname headername)
  if(NOT BGFX_${libname}_INCLUDE_DIRS)
    find_path(BGFX_${libname}_INCLUDE_DIRS NAMES ${headername} PATHS ${SEARCH_PATH}/include NO_DEFAULT_PATH)
  endif()
  if(NOT BGFX_${libname}_LIBRARY)
    find_library(BGFX_${libname}_LIBRARY_RELEASE NAMES ${shortname}Release PATHS ${SEARCH_PATH}/lib/ NO_DEFAULT_PATH)
    find_library(BGFX_${libname}_LIBRARY_DEBUG NAMES ${shortname}Debug PATHS ${SEARCH_PATH}/debug/lib/ NO_DEFAULT_PATH)
    get_filename_component(BGFX_${libname}_LIBRARY_RELEASE_DIR ${BGFX_${libname}_LIBRARY_RELEASE} DIRECTORY)
    get_filename_component(BGFX_${libname}_LIBRARY_DEBUG_DIR ${BGFX_${libname}_LIBRARY_DEBUG} DIRECTORY)
    select_library_configurations(BGFX_${libname})
    set(BGFX_${libname}_LIBRARY ${BGFX_${libname}_LIBRARY} CACHE STRING "")
  endif()
  if (BGFX_${libname}_LIBRARY AND BGFX_${libname}_INCLUDE_DIRS)
    set(BGFX_${libname}_FOUND TRUE BOOL)
    list(APPEND BGFX_INCLUDE_DIRS ${BGFX_${libname}_INCLUDE_DIRS})
    list(APPEND BGFX_LIBRARIES ${BGFX_${libname}_LIBRARY})
    list(APPEND BGFX_LIBRARY_DIRS ${BGFX_${libname}_LIBRARY_RELEASE_DIR} ${BGFX_${libname}_LIBRARY_DEBUG_DIR})
  endif()
endmacro(BGFX_FIND)

BGFX_FIND(bgfx bgfx bgfx/bgfx.h)

if (BGFX_bgfx_FOUND)
  list(REMOVE_DUPLICATES BGFX_INCLUDE_DIRS)
  list(REMOVE_DUPLICATES BGFX_LIBRARY_DIRS)
  set(BGFX_bgfx_VERSION "" CACHE STRING "")

  append_dependencies(BGFX_DEPS_LIBRARY_RELEASE NAMES "")
  append_dependencies(BGFX_DEPS_LIBRARY_DEBUG   NAMES "" DEBUG)
  if(BGFX_DEPS_LIBRARY_RELEASE OR BGFX_DEPS_LIBRARY_DEBUG)
    select_library_configurations(BGFX_DEPS)
    list(APPEND BGFX_LIBRARIES ${BGFX_DEPS_LIBRARY})
  endif()

  set(BGFX_LIBRARY ${BGFX_LIBRARIES})

  set(BGFX_FOUND TRUE CACHE BOOL "")
  set(BGFX_LIBRARIES ${BGFX_LIBRARIES} CACHE STRING "")
  set(BGFX_INCLUDE_DIRS ${BGFX_INCLUDE_DIRS} CACHE STRING "")
  set(BGFX_LIBRARY_DIRS ${BGFX_LIBRARY_DIRS} CACHE STRING "")
endif()

find_package_handle_standard_args(BGFX REQUIRED_VARS BGFX_LIBRARIES BGFX_LIBRARY_DIRS BGFX_INCLUDE_DIRS)

endif()
