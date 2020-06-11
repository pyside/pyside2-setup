include(CMakeParseArguments)

macro(set_python_shared_library_suffix)
  set(PYTHON_SHARED_LIBRARY_SUFFIX "${PYTHON_CONFIG_SUFFIX}")

  # Append a "v" to disambiguate the python version and the shiboken version in the
  # shared library file name.
  if (APPLE AND PYTHON_VERSION_MAJOR EQUAL 2)
      set(PYTHON_SHARED_LIBRARY_SUFFIX "${PYTHON_SHARED_LIBRARY_SUFFIX}v")
  endif()
endmacro()

macro(set_limited_api)
    if (WIN32 AND NOT EXISTS "${PYTHON_LIMITED_LIBRARIES}")
        message(FATAL_ERROR "The Limited API was enabled, but ${PYTHON_LIMITED_LIBRARIES} was not found!")
    endif()
    message(STATUS "******************************************************")
    message(STATUS "** Limited API enabled ${PYTHON_LIMITED_LIBRARIES}")
    message(STATUS "******************************************************")
endmacro()

macro(set_debug_build)
    set(SHIBOKEN_BUILD_TYPE "Debug")

    if(NOT PYTHON_DEBUG_LIBRARIES)
        message(WARNING "Python debug shared library not found; \
            assuming python was built with shared library support disabled.")
    endif()

    if(NOT PYTHON_WITH_DEBUG)
        message(WARNING "Compiling shiboken2 with debug enabled, \
            but the python executable was not compiled with debug support.")
    else()
        set(SBK_PKG_CONFIG_PY_DEBUG_DEFINITION " -DPy_DEBUG")
    endif()

    if (PYTHON_WITH_COUNT_ALLOCS)
        set(SBK_PKG_CONFIG_PY_DEBUG_DEFINITION "${SBK_PKG_CONFIG_PY_DEBUG_DEFINITION} -DCOUNT_ALLOCS")
    endif()
endmacro()

macro(setup_sanitize_address)
    # Currently this does not check that the clang / gcc version used supports Address sanitizer,
    # so once again, use at your own risk.
    add_compile_options("-fsanitize=address" "-g" "-fno-omit-frame-pointer")
    # We need to add the sanitize address option to all linked executables / shared libraries
    # so that proper sanitizer symbols are linked in.
    #
    # Note that when running tests, you may need to set an additional environment variable
    # in set_tests_properties for shiboken2 / pyside tests, or exported in your shell. Address
    # sanitizer will tell you what environment variable needs to be exported. For example:
    # export DYLD_INSERT_LIBRARIES=/Applications/Xcode.app/Contents/Developer/Toolchains/
    #   ./XcodeDefault.xctoolchain/usr/lib/clang/8.1.0/lib/darwin/libclang_rt.asan_osx_dynamic.dylib
    set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_STANDARD_LIBRARIES} -fsanitize=address")
endmacro()

macro(set_cmake_cxx_flags)
if(MSVC)
    # Qt5: this flag has changed from /Zc:wchar_t- in Qt4.X
    set(CMAKE_CXX_FLAGS "/Zc:wchar_t /GR /EHsc /DWIN32 /D_WINDOWS /D_SCL_SECURE_NO_WARNINGS")
    #set(CMAKE_CXX_FLAGS "/Zc:wchar_t /GR /EHsc /DNOCOLOR /DWIN32 /D_WINDOWS /D_SCL_SECURE_NO_WARNINGS") # XXX
else()
    if(CMAKE_HOST_UNIX AND NOT CYGWIN)
        add_definitions(-fPIC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fvisibility=hidden -Wno-strict-aliasing")
    endif()
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    option(ENABLE_GCC_OPTIMIZATION "Enable specific GCC flags to optimization library \
        size and performance. Only available on Release Mode" 0)
    if(ENABLE_GCC_OPTIMIZATION)
        set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -Os -Wl,-O1")
        if(NOT CMAKE_HOST_APPLE)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--hash-style=gnu")
        endif()
    endif()
    if(CMAKE_HOST_APPLE)
        # ALTERNATIVE_QT_INCLUDE_DIR is deprecated, because CMake takes care of finding the proper
        # include folders using the qmake found in the environment. Only use it for now in case
        # something goes wrong with the cmake process.
        if(ALTERNATIVE_QT_INCLUDE_DIR AND NOT QT_INCLUDE_DIR)
            set(QT_INCLUDE_DIR ${ALTERNATIVE_QT_INCLUDE_DIR})
        endif()
    endif()
endif()

endmacro()

macro(set_python_site_packages)
    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} -c "if True:
            from distutils import sysconfig
            from os.path import sep
            print(sysconfig.get_python_lib(1, 0, prefix='${CMAKE_INSTALL_PREFIX}').replace(sep, '/'))
            "
        OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT PYTHON_SITE_PACKAGES)
        message(FATAL_ERROR "Could not detect Python module installation directory.")
    elseif (APPLE)
        message(STATUS "!!! The generated bindings will be installed on ${PYTHON_SITE_PACKAGES}, \
            is it right!?")
    endif()
endmacro()

macro(set_python_config_suffix)
  if (PYTHON_VERSION_MAJOR EQUAL 2)
      set(PYTHON_CONFIG_SUFFIX "-python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}")
      if (PYTHON_EXTENSION_SUFFIX)
          set(PYTHON_CONFIG_SUFFIX "${PYTHON_CONFIG_SUFFIX}${PYTHON_EXTENSION_SUFFIX}")
      endif()
  elseif (PYTHON_VERSION_MAJOR EQUAL 3)
      if (PYTHON_LIMITED_API)
          if(WIN32)
              set(PYTHON_EXTENSION_SUFFIX "")
          else()
              set(PYTHON_EXTENSION_SUFFIX ".abi3")
          endif()
          set(PYTHON_CONFIG_SUFFIX ".abi3")
      else()
          set(PYTHON_CONFIG_SUFFIX "${PYTHON_EXTENSION_SUFFIX}")
      endif()
  endif()
endmacro()

macro(setup_clang)
    set(CLANG_DIR "")
    set(CLANG_DIR_SOURCE "")

    set(clang_not_found_message "Unable to detect CLANG location by checking LLVM_INSTALL_DIR, \
                                 CLANG_INSTALL_DIR or running llvm-config.")

    if (DEFINED ENV{LLVM_INSTALL_DIR})
        set(CLANG_DIR $ENV{LLVM_INSTALL_DIR})
        string(REPLACE "_ARCH_" "${PYTHON_ARCH}" CLANG_DIR "${CLANG_DIR}")
        set(CLANG_DIR_SOURCE "LLVM_INSTALL_DIR")
    elseif (DEFINED ENV{CLANG_INSTALL_DIR})
        set(CLANG_DIR $ENV{CLANG_INSTALL_DIR})
        string(REPLACE "_ARCH_" "${PYTHON_ARCH}" CLANG_DIR "${CLANG_DIR}")
        set(CLANG_DIR_SOURCE "CLANG_INSTALL_DIR")
    else ()
        if (NOT LLVM_CONFIG)
            get_llvm_config()
        endif()
        set(CLANG_DIR_SOURCE "${LLVM_CONFIG}")
        if ("${CLANG_DIR_SOURCE}" STREQUAL "")
            message(FATAL_ERROR "${clang_not_found_message}")
        endif()

        EXEC_PROGRAM("${LLVM_CONFIG}" ARGS "--prefix" OUTPUT_VARIABLE CLANG_DIR)
        if (NOT "${CLANG_DIR}" STREQUAL "")
            EXEC_PROGRAM("${LLVM_CONFIG}" ARGS "--version" OUTPUT_VARIABLE CLANG_VERSION)
            if (CLANG_VERSION VERSION_LESS 3.9)
                message(FATAL_ERROR "libclang version 3.9 or higher is required \
                    (${LLVM_CONFIG} detected ${CLANG_VERSION} at ${CLANG_DIR}).")
            endif()
        endif()
    endif()

    if ("${CLANG_DIR}" STREQUAL "")
        message(FATAL_ERROR "${clang_not_found_message}")
    elseif (NOT IS_DIRECTORY ${CLANG_DIR})
        message(FATAL_ERROR "${CLANG_DIR} detected by ${CLANG_DIR_SOURCE} does not exist.")
    endif()

    # The non-development Debian / Ubuntu packages (e.g. libclang1-6.0) do not ship a
    # libclang.so symlink, but only libclang-6.0.so.1 and libclang.so.1 (adjusted for version number).
    # Thus searching for libclang would not succeed.
    # The "libclang.so" symlink is shipped as part of the development package (libclang-6.0-dev) which
    # we need anyway because of the headers. Thus we will search for libclang.so.1 also, and complain
    # about the headers not being found in a check further down. This is more friendly to the user,
    # so they don't scratch their head thinking that they have already installed the necessary package.
    set(CLANG_LIB_NAMES clang libclang.so libclang.so.1)
    if(MSVC)
        set(CLANG_LIB_NAMES libclang)
    endif()

    find_library(CLANG_LIBRARY NAMES ${CLANG_LIB_NAMES} HINTS ${CLANG_DIR}/lib)
    if (NOT EXISTS ${CLANG_LIBRARY})
        string(REPLACE ";" ", " CLANG_LIB_NAMES_STRING "${CLANG_LIB_NAMES}")
        message(FATAL_ERROR "Unable to find the Clang library in ${CLANG_DIR}.\
            Names tried: ${CLANG_LIB_NAMES_STRING}.")
    endif()

    message(STATUS "CLANG: ${CLANG_DIR}, ${CLANG_LIBRARY} detected by ${CLANG_DIR_SOURCE}")

    set(CLANG_EXTRA_INCLUDES ${CLANG_DIR}/include)
    set(CLANG_EXTRA_LIBRARIES ${CLANG_LIBRARY})

    # Check if one of the required clang headers is found. Error out early at CMake time instead of
    # compile time if not found.
    # It can happen that a user uses a distro-provided libclang.so, but no development header package
    # was installed (e.g. libclang-6.0-dev on Ubuntu).
    set(CMAKE_REQUIRED_INCLUDES ${CLANG_EXTRA_INCLUDES})
    set(CLANG_HEADER_FILE_TO_CHECK "clang-c/Index.h")
    check_include_file_cxx(${CLANG_HEADER_FILE_TO_CHECK} CLANG_INCLUDE_FOUND)
    unset(CMAKE_REQUIRED_INCLUDES)
    if (NOT CLANG_INCLUDE_FOUND)
        # Need to unset so that when installing the package, CMake doesn't complain that the header
        # still isn't found.
        unset(CLANG_INCLUDE_FOUND CACHE)
        message(FATAL_ERROR "Unable to find required Clang header file ${CLANG_HEADER_FILE_TO_CHECK} \
            in ${CLANG_DIR}/include. Perhaps you forgot to install the clang development header \
            package? (e.g. libclang-6.0-dev)")
    endif()
endmacro()

macro(set_quiet_build)
    # Don't display "up-to-date / install" messages when installing, to reduce visual clutter.
    set(CMAKE_INSTALL_MESSAGE NEVER)
    # Override message not to display info messages when doing a quiet build.
    function(message)
        list(GET ARGV 0 MessageType)
        if (MessageType STREQUAL FATAL_ERROR OR
            MessageType STREQUAL SEND_ERROR OR
            MessageType STREQUAL WARNING OR
            MessageType STREQUAL AUTHOR_WARNING)
                list(REMOVE_AT ARGV 0)
                _message(${MessageType} "${ARGV}")
        endif()
    endfunction()
endmacro()

macro(get_python_extension_suffix)
  # Result of importlib.machinery.EXTENSION_SUFFIXES depends on the platform,
  # but generally looks something like:
  # ['.cpython-38-x86_64-linux-gnu.so', '.abi3.so', '.so']
  # We pick the first most detailed one.

  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "if True:
       import re
       try:
           from importlib import machinery
           first_suffix = machinery.EXTENSION_SUFFIXES[0]
       except (AttributeError, ImportError):
           import imp
           first_suffix = imp.get_suffixes()[0][0]
       res = re.search(r'^(.+)\\.', first_suffix)
       if res:
           first_suffix = res.group(1)
       else:
           first_suffix = ''
       print(first_suffix)
       "
    OUTPUT_VARIABLE PYTHON_EXTENSION_SUFFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "PYTHON_EXTENSION_SUFFIX: " ${PYTHON_EXTENSION_SUFFIX})
endmacro()

macro(get_llvm_config)
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "if True:
       import os
       import sys
       sys.path.append(os.path.realpath(os.path.join('${CMAKE_CURRENT_LIST_DIR}', '..', '..')))
       from build_scripts.utils import find_llvm_config
       llvmConfig = find_llvm_config()
       if llvmConfig:
           print(llvmConfig)
       "
    OUTPUT_VARIABLE LLVM_CONFIG
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "LLVM_CONFIG:             " ${LLVM_CONFIG})
endmacro()

macro(get_python_arch)
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "if True:
       import sys
       print('64' if sys.maxsize > 2**31-1 else '32')
       "
    OUTPUT_VARIABLE PYTHON_ARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "PYTHON_ARCH:             " ${PYTHON_ARCH})
endmacro()

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
            import os
            for lib in '${PYTHON_LIBRARIES}'.split(';'):
                if '/' in lib and os.path.isfile(lib):
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
