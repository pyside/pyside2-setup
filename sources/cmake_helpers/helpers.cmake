macro(collect_essential_modules)
# Collect all essential modules.
# note: the order of this list is relevant for dependencies.
# For instance: Qt5Printsupport must come before Qt5WebKitWidgets.
set(ALL_ESSENTIAL_MODULES
    Core
    Gui
    Widgets
    PrintSupport
    Sql
    Network
    Test
    Concurrent)
if(UNIX AND NOT APPLE)
    list(APPEND ALL_ESSENTIAL_MODULES X11Extras)
endif()
if(WIN32)
    list(APPEND ALL_ESSENTIAL_MODULES WinExtras)
endif()
if(APPLE)
    list(APPEND ALL_ESSENTIAL_MODULES MacExtras)
endif()
endmacro()

macro(collect_optional_modules)
# Collect all optional modules.
set(ALL_OPTIONAL_MODULES
    Xml
    Help Multimedia
    MultimediaWidgets
    OpenGL
    OpenGLFunctions
    OpenGLWidgets
    Positioning
    Location
    Qml
    Quick
    QuickControls2
    QuickWidgets
    RemoteObjects
    Scxml
    Sensors
    SerialPort
    TextToSpeech
    Charts
    Svg
    DataVisualization)
find_package(Qt${QT_MAJOR_VERSION}UiTools)
if(Qt${QT_MAJOR_VERSION}UiTools_FOUND)
    list(APPEND ALL_OPTIONAL_MODULES UiTools)
else()
    set(DISABLE_QtUiTools 1)
endif()
if(WIN32)
    list(APPEND ALL_OPTIONAL_MODULES AxContainer)
endif()
list(APPEND ALL_OPTIONAL_MODULES WebChannel WebEngineCore WebEngine WebEngineWidgets WebSockets)
if (Qt${QT_MAJOR_VERSION}Core_VERSION VERSION_GREATER 5.9.3) # Depending on fixes in Qt3D
    list(APPEND ALL_OPTIONAL_MODULES 3DCore 3DRender 3DInput 3DLogic 3DAnimation 3DExtras)
endif()
endmacro()

macro(check_os)
set(ENABLE_X11 "0")
set(ENABLE_MAC "0")
set(ENABLE_WIN "0")
set(ENABLE_SIMULATOR "0")

if(CMAKE_HOST_APPLE)
    set(ENABLE_MAC "1")
    set(AUTO_OS "mac")
elseif(CMAKE_HOST_WIN32)
    set(ENABLE_WIN "1")
    set(AUTO_OS "win")
elseif(CMAKE_HOST_UNIX)
    set(ENABLE_X11 "1")
    set(AUTO_OS "x11")
else()
    message(FATAL_ERROR "OS not supported")
endif()
endmacro()

macro(use_protected_as_public_hack)
# 2017-04-24 The protected hack can unfortunately not be disabled, because
# Clang does produce linker errors when we disable the hack.
# But the ugly workaround in Python is replaced by a shiboken change.
if(WIN32 OR DEFINED AVOID_PROTECTED_HACK)
    message(STATUS "PySide2 will be generated avoiding the protected hack!")
    set(GENERATOR_EXTRA_FLAGS ${GENERATOR_EXTRA_FLAGS} --avoid-protected-hack)
    add_definitions(-DAVOID_PROTECTED_HACK)
else()
    message(STATUS "PySide will be generated using the protected hack!")
endif()
endmacro()

macro(remove_skipped_modules)
# Removing from the MODULES list the items that were defined with
# -DSKIP_MODULES on command line
if (SKIP_MODULES)
    foreach(s ${SKIP_MODULES})
        list(REMOVE_ITEM MODULES ${s})
    endforeach()
endif()

foreach(m ${MODULES})
    COLLECT_MODULE_IF_FOUND(${m})
    list(FIND all_module_shortnames ${m} is_module_collected)
    # If the module was collected, remove it from disabled modules list.
    if (NOT is_module_collected EQUAL -1)
        list(REMOVE_ITEM DISABLED_MODULES ${m})
    endif()
endforeach()
endmacro()

macro(COLLECT_MODULE_IF_FOUND shortname)
    set(name "Qt${QT_MAJOR_VERSION}${shortname}")
    set(_qt_module_name "${name}")
    if ("${shortname}" STREQUAL "OpenGLFunctions")
        set(_qt_module_name "Qt${QT_MAJOR_VERSION}Gui")
    endif()
    # Determine essential/optional/missing
    set(module_state "missing")
    list(FIND ALL_ESSENTIAL_MODULES "${shortname}" essentialIndex)
    if(${essentialIndex} EQUAL -1)
        list(FIND ALL_OPTIONAL_MODULES "${shortname}" optionalIndex)
        if(NOT ${optionalIndex} EQUAL -1)
            set(module_state "optional")
        endif()
    else()
        set(module_state "essential")
    endif()

    # Silence warnings when optional packages are not found when doing a quiet build.
    set(quiet_argument "")
    if (QUIET_BUILD AND "${module_state}" STREQUAL "optional")
        set(quiet_argument "QUIET")
    endif()

    find_package(${_qt_module_name} ${quiet_argument})
    # If package is found, _name_found will be equal to 1
    set(_name_found "${_qt_module_name}_FOUND")
    # _name_dir will keep the path to the directory where the CMake rules were found
    # e.g: ~/qt5.9-install/qtbase/lib/cmake/Qt5Core or /usr/lib64/cmake/Qt5Core
    set(_name_dir "${_qt_module_name}_DIR")
    # Qt5Core will set the base path to check if all the modules are on the same
    # directory, to avoid CMake looking in another path.
    # This will be saved in a global variable at the beginning of the modules
    # collection process.
    string(FIND "${name}" "Qt${QT_MAJOR_VERSION}Core" qtcore_found)
    if(("${qtcore_found}" GREATER "0") OR ("${qtcore_found}" EQUAL "0"))
        get_filename_component(_core_abs_dir "${${_name_dir}}/../" ABSOLUTE)
        # Setting the absolute path where the Qt5Core was found
        # e.g: ~/qt5.9-install/qtbase/lib/cmake or /usr/lib64/cmake
        message(STATUS "CORE_ABS_DIR:" ${_core_abs_dir})
    endif()

    # Getting the absolute path for each module where the CMake was found, to
    # compare it with CORE_ABS_DIR and check if they are in the same source directory
    # e.g: ~/qt5.9-install/qtbase/lib/cmake/Qt5Script or /usr/lib64/cmake/Qt5Script
    get_filename_component(_module_dir "${${_name_dir}}" ABSOLUTE)
    string(FIND "${_module_dir}" "${_core_abs_dir}" found_basepath)

    # If the module was found, and also the module path is the same as the
    # Qt5Core base path, we will generate the list with the modules to be installed
    set(looked_in_message ". Looked in: ${${_name_dir}}")
    if("${${_name_found}}" AND (("${found_basepath}" GREATER "0") OR ("${found_basepath}" EQUAL "0")))
        message(STATUS "${module_state} module ${name} found (${ARGN})${looked_in_message}")
        # record the shortnames for the tests
        list(APPEND all_module_shortnames ${shortname})
        # Build Qt 5 compatibility variables
        if(${QT_MAJOR_VERSION} GREATER_EQUAL 6)
            get_target_property(Qt6${shortname}_INCLUDE_DIRS Qt6::${shortname}
                                INTERFACE_INCLUDE_DIRECTORIES)
            get_target_property(Qt6${shortname}_PRIVATE_INCLUDE_DIRS
                                Qt6::${shortname}Private
                                INTERFACE_INCLUDE_DIRECTORIES)
            set(Qt6${shortname}_LIBRARIES Qt::${shortname})
        endif()
    else()
        if("${module_state}" STREQUAL "optional")
            message(STATUS "optional module ${name} skipped${looked_in_message}")
        elseif("${module_state}" STREQUAL "essential")
            message(STATUS "skipped module ${name} is essential!\n"
                           "   We do not guarantee that all tests are working.${looked_in_message}")
        else()
            message(FATAL_ERROR "module ${name} MISSING${looked_in_message}")
        endif()
    endif()
endmacro()

macro(compute_config_py_values
      full_version_var_name
        )
    string(TIMESTAMP PACKAGE_BUILD_DATE "%Y-%m-%dT%H:%M:%S+00:00" UTC)
    if (PACKAGE_BUILD_DATE)
        set(PACKAGE_BUILD_DATE "__build_date__ = '${PACKAGE_BUILD_DATE}'")
    endif()

    if (PACKAGE_SETUP_PY_PACKAGE_VERSION)
        set(PACKAGE_SETUP_PY_PACKAGE_VERSION_ASSIGNMENT "__setup_py_package_version__ = '${PACKAGE_SETUP_PY_PACKAGE_VERSION}'")
        set(FINAL_PACKAGE_VERSION ${PACKAGE_SETUP_PY_PACKAGE_VERSION})
    else()
        set(FINAL_PACKAGE_VERSION ${${full_version_var_name}})
    endif()

    if (PACKAGE_SETUP_PY_PACKAGE_TIMESTAMP)
        set(PACKAGE_SETUP_PY_PACKAGE_TIMESTAMP_ASSIGNMENT "__setup_py_package_timestamp__ = '${PACKAGE_SETUP_PY_PACKAGE_TIMESTAMP}'")
    else()
        set(PACKAGE_SETUP_PY_PACKAGE_TIMESTAMP_ASSIGNMENT "")
    endif()

    find_package(Git)
    if(GIT_FOUND)
        # Check if current source folder is inside a git repo, so that commit information can be
        # queried.
        execute_process(
          COMMAND ${GIT_EXECUTABLE} rev-parse --git-dir
          OUTPUT_VARIABLE PACKAGE_SOURCE_IS_INSIDE_REPO
          ERROR_QUIET
          OUTPUT_STRIP_TRAILING_WHITESPACE)

        if(PACKAGE_SOURCE_IS_INSIDE_REPO)
            # Force git dates to be UTC-based.
            set(ENV{TZ} UTC)
            execute_process(
              COMMAND ${GIT_EXECUTABLE} --no-pager show --date=format-local:%Y-%m-%dT%H:%M:%S+00:00 -s --format=%cd HEAD
              OUTPUT_VARIABLE PACKAGE_BUILD_COMMIT_DATE
              OUTPUT_STRIP_TRAILING_WHITESPACE)
            if(PACKAGE_BUILD_COMMIT_DATE)
                set(PACKAGE_BUILD_COMMIT_DATE "__build_commit_date__ = '${PACKAGE_BUILD_COMMIT_DATE}'")
            endif()
            unset(ENV{TZ})

            execute_process(
              COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
              OUTPUT_VARIABLE PACKAGE_BUILD_COMMIT_HASH
              OUTPUT_STRIP_TRAILING_WHITESPACE)
            if(PACKAGE_BUILD_COMMIT_HASH)
                set(PACKAGE_BUILD_COMMIT_HASH "__build_commit_hash__ = '${PACKAGE_BUILD_COMMIT_HASH}'")
            endif()

            execute_process(
              COMMAND ${GIT_EXECUTABLE} describe HEAD
              OUTPUT_VARIABLE PACKAGE_BUILD_COMMIT_HASH_DESCRIBED
              OUTPUT_STRIP_TRAILING_WHITESPACE)
            if(PACKAGE_BUILD_COMMIT_HASH_DESCRIBED)
                set(PACKAGE_BUILD_COMMIT_HASH_DESCRIBED "__build_commit_hash_described__ = '${PACKAGE_BUILD_COMMIT_HASH_DESCRIBED}'")
            endif()

        endif()
    endif()

endmacro()

# Creates a new target called "${library_name}_generator" which
# depends on the mjb_rejected_classes.log file generated by shiboken.
# This target is added as a dependency to ${library_name} target.
# This file's timestamp informs cmake when the last generation was
# done, without force-updating the timestamps of the generated class
# cpp files.
# In practical terms this means that changing some injection code in
# an xml file that modifies only one specific class cpp file, will
# not force rebuilding all the cpp files, and thus allow for better
# incremental builds.
macro(create_generator_target library_name)
    add_custom_target(${library_name}_generator DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/mjb_rejected_classes.log")
    add_dependencies(${library_name} ${library_name}_generator)
endmacro()
