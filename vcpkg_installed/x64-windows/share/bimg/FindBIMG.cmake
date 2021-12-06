# Distributed under the OSI-approved BSD 3-Clause License.
#
#.rst:
# FindBIMG
# --------
#
# Find the BIMG libraries
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# The following variables will be defined:
#
#  ``BIMG_FOUND``
#    True if BIMG found on the local system
#
#  ``BIMG_INCLUDE_DIRS``
#    Location of BIMG header files
#
#  ``BIMG_LIBRARY_DIRS``
#    Location of BIMG libraries
#
#  ``BIMG_LIBRARIES``
#    List of the BIMG libraries found
#
#

include(FindPackageHandleStandardArgs)
include(SelectLibraryConfigurations)
include(CMakeFindDependencyMacro)

if(NOT BIMG_FOUND)

# Compute the installation path relative to this file.
get_filename_component(SEARCH_PATH "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(SEARCH_PATH "${SEARCH_PATH}" PATH)
get_filename_component(SEARCH_PATH "${SEARCH_PATH}" PATH)
if(SEARCH_PATH STREQUAL "/")
  set(SEARCH_PATH "")
endif()

set(BIMG_VERSION "")

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
            find_library(BIMG_DEPENDENCY_${lib_name}_${config} NAMES "${lib_name}" PATHS "${path}" NO_DEFAULT_PATH)
            # if not found there, must be a system dependency, so look elsewhere
            find_library(BIMG_DEPENDENCY_${lib_name}_${config} NAMES "${lib_name}" REQUIRED)
            list(APPEND ${out} "${BIMG_DEPENDENCY_${lib_name}_${config}}")
        endif()
    endforeach()
    set("${out}" "${${out}}" PARENT_SCOPE)
endfunction()

macro(BIMG_FIND libname shortname headername)
  if(NOT BIMG_${libname}_INCLUDE_DIRS)
    find_path(BIMG_${libname}_INCLUDE_DIRS NAMES ${headername} PATHS ${SEARCH_PATH}/include NO_DEFAULT_PATH)
  endif()
  if(NOT BIMG_${libname}_LIBRARY)
    find_library(BIMG_${libname}_LIBRARY_RELEASE NAMES ${shortname}Release PATHS ${SEARCH_PATH}/lib/ NO_DEFAULT_PATH)
    find_library(BIMG_${libname}_LIBRARY_DEBUG NAMES ${shortname}Debug PATHS ${SEARCH_PATH}/debug/lib/ NO_DEFAULT_PATH)
    get_filename_component(BIMG_${libname}_LIBRARY_RELEASE_DIR ${BIMG_${libname}_LIBRARY_RELEASE} DIRECTORY)
    get_filename_component(BIMG_${libname}_LIBRARY_DEBUG_DIR ${BIMG_${libname}_LIBRARY_DEBUG} DIRECTORY)
    select_library_configurations(BIMG_${libname})
    set(BIMG_${libname}_LIBRARY ${BIMG_${libname}_LIBRARY} CACHE STRING "")
  endif()
  if (BIMG_${libname}_LIBRARY AND BIMG_${libname}_INCLUDE_DIRS)
    set(BIMG_${libname}_FOUND TRUE BOOL)
    list(APPEND BIMG_INCLUDE_DIRS ${BIMG_${libname}_INCLUDE_DIRS})
    list(APPEND BIMG_LIBRARIES ${BIMG_${libname}_LIBRARY})
    list(APPEND BIMG_LIBRARY_DIRS ${BIMG_${libname}_LIBRARY_RELEASE_DIR} ${BIMG_${libname}_LIBRARY_DEBUG_DIR})
  endif()
endmacro(BIMG_FIND)

BIMG_FIND(bimg bimg bimg/bimg.h)
BIMG_FIND(bimg_decode bimg_decode bimg/decode.h)
BIMG_FIND(bimg_encode bimg_encode bimg/encode.h)

if (BIMG_bimg_FOUND)
  list(REMOVE_DUPLICATES BIMG_INCLUDE_DIRS)
  list(REMOVE_DUPLICATES BIMG_LIBRARY_DIRS)
  set(BIMG_bimg_VERSION "" CACHE STRING "")

  append_dependencies(BIMG_DEPS_LIBRARY_RELEASE NAMES "")
  append_dependencies(BIMG_DEPS_LIBRARY_DEBUG   NAMES "" DEBUG)
  if(BIMG_DEPS_LIBRARY_RELEASE OR BIMG_DEPS_LIBRARY_DEBUG)
    select_library_configurations(BIMG_DEPS)
    list(APPEND BIMG_LIBRARIES ${BIMG_DEPS_LIBRARY})
  endif()

  set(BIMG_LIBRARY ${BIMG_LIBRARIES})

  set(BIMG_FOUND TRUE CACHE BOOL "")
  set(BIMG_LIBRARIES ${BIMG_LIBRARIES} CACHE STRING "")
  set(BIMG_INCLUDE_DIRS ${BIMG_INCLUDE_DIRS} CACHE STRING "")
  set(BIMG_LIBRARY_DIRS ${BIMG_LIBRARY_DIRS} CACHE STRING "")
endif()

find_package_handle_standard_args(BIMG REQUIRED_VARS BIMG_LIBRARIES BIMG_LIBRARY_DIRS BIMG_INCLUDE_DIRS)

endif()
