include(CMakeParseArguments)

macro(shiboken_parse_all_arguments prefix type flags options multiopts)
    cmake_parse_arguments(${prefix} "${flags}" "${options}" "${multiopts}" ${ARGN})
    if(DEFINED ${prefix}_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown arguments were passed to ${type} (${${prefix}_UNPARSED_ARGUMENTS}).")
    endif()
endmacro()

macro(shiboken_check_if_limited_api)
    # On Windows, PYTHON_LIBRARIES can be a list. Example:
    #    optimized;C:/Python36/libs/python36.lib;debug;C:/Python36/libs/python36_d.lib
    # On other platforms, this result is not used at all.
    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} -c "if True:
            for lib in '${PYTHON_LIBRARIES}'.split(';'):
                if '/' in lib:
                    prefix, py = lib.rsplit('/', 1)
                    if py.startswith('python3'):
                        print(prefix + '/python3.lib')
                        break
            "
        OUTPUT_VARIABLE PYTHON_LIMITED_LIBRARIES
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(FORCE_LIMITED_API STREQUAL "yes")
        if (${PYTHON_VERSION_MAJOR} EQUAL 3 AND ${PYTHON_VERSION_MINOR} GREATER 4)
            # GREATER_EQUAL is available only from cmake 3.7 on. We mean python 3.5 .
            set(PYTHON_LIMITED_API 1)
        endif()
        if(WIN32)
            if (${PYTHON_VERSION_MAJOR} EQUAL 3 AND ${PYTHON_VERSION_MINOR} GREATER 4)
                # PYSIDE-560: XXX maybe add an option to setup.py as override
                set(SHIBOKEN_PYTHON_LIBRARIES ${PYTHON_LIMITED_LIBRARIES})
            endif()
        endif()
    endif()
endmacro()


macro(shiboken_find_required_python)
    if(${ARGC} GREATER 0)
        find_package(PythonInterp ${ARGV0} REQUIRED)
        find_package(PythonLibs ${ARGV0} REQUIRED)
    else()
        # If no version is specified, just use any interpreter that can be found (from PATH).
        # This is useful for super-project builds, so that the default system interpeter
        # gets picked up (e.g. /usr/bin/python and not /usr/bin/python2.7).
        find_package(PythonInterp REQUIRED)
        find_package(PythonLibs REQUIRED)
    endif()
    shiboken_validate_python_version()

    set(SHIBOKEN_PYTHON_INTERPRETER "${PYTHON_EXECUTABLE}")
    set_property(GLOBAL PROPERTY SHIBOKEN_PYTHON_INTERPRETER "${PYTHON_EXECUTABLE}")
endmacro()

macro(shiboken_validate_python_version)
    if((PYTHON_VERSION_MAJOR EQUAL "2" AND PYTHON_VERSION_MINOR LESS "7") OR
       (PYTHON_VERSION_MAJOR EQUAL "3" AND PYTHON_VERSION_MINOR LESS "5"))
            message(FATAL_ERROR
                   "Shiboken requires Python 2.7+ or Python 3.5+.")
    endif()
endmacro()

macro(shiboken_compute_python_includes)
    shiboken_parse_all_arguments(
        "SHIBOKEN_COMPUTE_INCLUDES" "shiboken_compute_python_includes"
        "IS_CALLED_FROM_EXPORT" "" "" ${ARGN})


    # If the installed shiboken config file is used,
    # append the found Python include dirs as an interface property on the libshiboken target.
    # This needs to be dynamic because the user of the library might have python installed
    # in a different path than when shiboken was originally built.
    # Otherwise if shiboken is currently being built itself (either as standalone, or super project
    # build) append the include dirs as PUBLIC.
    if (SHIBOKEN_COMPUTE_INCLUDES_IS_CALLED_FROM_EXPORT)
        #TODO target_include_directories works on imported targets only starting with v3.11.0.
        set_property(TARGET Shiboken2::libshiboken
                     APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${PYTHON_INCLUDE_DIRS})
    else()
        target_include_directories(libshiboken
                                   PUBLIC $<BUILD_INTERFACE:${PYTHON_INCLUDE_DIRS}>)
    endif()


    set(SHIBOKEN_PYTHON_INCLUDE_DIRS "${PYTHON_INCLUDE_DIRS}")
    message(STATUS
            "SHIBOKEN_PYTHON_INCLUDE_DIRS computed to value: '${SHIBOKEN_PYTHON_INCLUDE_DIRS}'")
endmacro()

# Given a list of the following form:
#     optimized;C:/Python36/libs/python36.lib;debug;C:/Python36/libs/python36_d.lib
# choose the correpsonding library to use, based on the current configuration type.
function(shiboken_get_library_for_current_config library_list current_config out_var)
    list(FIND library_list "optimized" optimized_found)
    list(FIND library_list "general" general_found)
    list(FIND library_list "debug" debug_found)

    if (optimized_found OR general_found OR debug_found)
        # Iterate over library list to find the most appropriate library.
        foreach(token ${library_list})
            if(token STREQUAL "optimized" OR token STREQUAL "general")
                set(is_debug 0)
                set(token1 1)
                set(lib "")
            elseif(token STREQUAL "debug")
                set(is_debug 1)
                set(token1 1)
                set(lib "")
            elseif(EXISTS ${token})
                set(lib ${token})
                set(token2 1)
            else()
                set(token1 0)
                set(token2 0)
                set(lib "")
            endif()

            if(token1 AND token2)
                if((is_debug AND lib AND current_config STREQUAL "Debug")
                   OR (NOT is_debug AND lib AND (NOT current_config STREQUAL "Debug")))
                    set(${out_var} ${lib} PARENT_SCOPE)
                    return()
                endif()
            endif()
        endforeach()
    else()
        # No configuration specific libraries found, just set the original value.
        set(${out_var} "${library_list}" PARENT_SCOPE)
    endif()

endfunction()

macro(shiboken_compute_python_libraries)
    shiboken_parse_all_arguments(
        "SHIBOKEN_COMPUTE_LIBS" "shiboken_compute_python_libraries"
        "IS_CALLED_FROM_EXPORT" "" "" ${ARGN})

    if (NOT SHIBOKEN_PYTHON_LIBRARIES)
        set(SHIBOKEN_PYTHON_LIBRARIES "")
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(WIN32 AND NOT SHIBOKEN_PYTHON_LIBRARIES)
            set(SHIBOKEN_PYTHON_LIBRARIES ${PYTHON_DEBUG_LIBRARIES})
        endif()
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        if(WIN32 AND NOT SHIBOKEN_PYTHON_LIBRARIES)
            set(SHIBOKEN_PYTHON_LIBRARIES ${PYTHON_LIBRARIES})
        endif()
    endif()

    # If the resulting variable
    # contains a "debug;X;optimized;Y" list like described in shiboken_check_if_limited_api,
    # make sure to pick just one, so that the final generator expressions are valid.
    shiboken_get_library_for_current_config("${SHIBOKEN_PYTHON_LIBRARIES}" "${CMAKE_BUILD_TYPE}" "SHIBOKEN_PYTHON_LIBRARIES")

    if(APPLE)
        set(SHIBOKEN_PYTHON_LIBRARIES "-undefined dynamic_lookup")
    endif()

    # If the installed shiboken config file is used,
    # append the computed Python libraries as an interface property on the libshiboken target.
    # This needs to be dynamic because the user of the library might have python installed
    # in a different path than when shiboken was originally built.
    # Otherwise if shiboken is currently being built itself (either as standalone, or super project
    # build) append the libraries as PUBLIC.
    if (SHIBOKEN_COMPUTE_LIBS_IS_CALLED_FROM_EXPORT)
        #TODO target_link_libraries works on imported targets only starting with v3.11.0.
        set_property(TARGET Shiboken2::libshiboken
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${SHIBOKEN_PYTHON_LIBRARIES})
    else()
        target_link_libraries(libshiboken
                              PUBLIC $<BUILD_INTERFACE:${SHIBOKEN_PYTHON_LIBRARIES}>)
    endif()

    message(STATUS "SHIBOKEN_PYTHON_LIBRARIES computed to value: '${SHIBOKEN_PYTHON_LIBRARIES}'")
endmacro()

function(shiboken_check_if_built_and_target_python_are_compatible)
    if(NOT SHIBOKEN_PYTHON_VERSION_MAJOR STREQUAL PYTHON_VERSION_MAJOR)
        message(FATAL_ERROR "The detected Python major version is not \
compatible with the Python major version which was used when Shiboken was built.
Built with: '${SHIBOKEN_PYTHON_VERSION_MAJOR}.${SHIBOKEN_PYTHON_VERSION_MINOR}' \
Detected: '${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}'")
    else()
        if(NOT SHIBOKEN_PYTHON_LIMITED_API
           AND NOT SHIBOKEN_PYTHON_VERSION_MINOR STREQUAL PYTHON_VERSION_MINOR)
            message(FATAL_ERROR
                    "The detected Python minor version is not compatible with the Python minor \
version which was used when Shiboken was built. Consider building shiboken with \
FORCE_LIMITED_API set to '1', so that only the Python major version matters.
Built with: '${SHIBOKEN_PYTHON_VERSION_MAJOR}.${SHIBOKEN_PYTHON_VERSION_MINOR}' \
Detected: '${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}'")
        endif()
    endif()
endfunction()
