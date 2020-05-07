project(pyside2)

# Configure include based on platform
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/global.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/pyside2_global.h" @ONLY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
               "${CMAKE_CURRENT_BINARY_DIR}/__init__.py" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/_config.py.in"
               "${CMAKE_CURRENT_BINARY_DIR}/_config.py" @ONLY)
# typing support for mypy
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/py.typed.in"
               "${CMAKE_CURRENT_BINARY_DIR}/py.typed" @ONLY)

# Use absolute path instead of relative path, to avoid ninja build errors due to
# duplicate file dependency inconsistency.
set(pyside_version_relative_path "${CMAKE_CURRENT_SOURCE_DIR}/../pyside_version.py")
get_filename_component(pyside_version_path ${pyside_version_relative_path} ABSOLUTE)
configure_file("${pyside_version_path}"
               "${CMAKE_CURRENT_BINARY_DIR}/_git_pyside_version.py" @ONLY)

# qt.conf needs to be placed next to QtWebEngineProcess so that the executable uses the correct
# Prefix location leading to an existing icu data file. It is needed on Windows, Linux, and macOS
# non-framework build. In framework build, instead of using qt.conf, Bundle querying is used.
if (WIN32 OR (UNIX AND NOT APPLE) OR (APPLE AND NOT QtCore_is_framework))

    if (WIN32)
        # On Windows, the QtWebEngineProcess is directly located in the Prefix, due to not using
        # a "Qt" subfolder like on the other platforms.
        set(QT_CONF_PREFIX ".")
    else()
        # On Linux and non-framework macOS, the QtWebEngineProcess is in ./libexec, so prefix is one
        # level higher.
        set(QT_CONF_PREFIX "..")
    endif()

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/qt.conf.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/qt.conf" @ONLY)
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/__init__.py"
               "${CMAKE_CURRENT_BINARY_DIR}/support/__init__.py" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/generate_pyi.py"
               "${CMAKE_CURRENT_BINARY_DIR}/support/generate_pyi.py" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/deprecated.py"
               "${CMAKE_CURRENT_BINARY_DIR}/support/deprecated.py" COPYONLY)

# now compile all modules.
file(READ "${CMAKE_CURRENT_BINARY_DIR}/pyside2_global.h" pyside2_global_contents)

foreach(shortname IN LISTS all_module_shortnames)
    set(name "Qt${QT_MAJOR_VERSION}${shortname}")
    set(_qt_module_name "${name}")
    if ("${shortname}" STREQUAL "OpenGLFunctions")
        set(_qt_module_name "Qt${QT_MAJOR_VERSION}Gui")
    endif()
    HAS_QT_MODULE(${_qt_module_name}_FOUND Qt${shortname})

    # Create a module header consisting of pyside2_global.h and the module
    # include. Note: The contents of pyside2_global.h must be copied instead of
    # just #including it since shiboken will otherwise generate an
    # #include <pyside2_global.h> for the the type entries originating from it
    # (cf AbstractMetaBuilderPrivate::setInclude()).
    set(module_header "${CMAKE_CURRENT_BINARY_DIR}/Qt${shortname}_global.h")
    set(module_header_content "")
    set(pre_header "${CMAKE_CURRENT_BINARY_DIR}/Qt${shortname}/Qt${shortname}_global.pre.h")
    if(EXISTS "${pre_header}")
        file(READ "${pre_header}" pre_header_content)
        set(module_header_content "${pre_header_content}")
    endif()
    set(module_header_content "${module_header_content}\n${pyside2_global_contents}")
    # AxContainer/AxServer from Active Qt do not have module headers
    if(NOT "${shortname}" STREQUAL "AxContainer" AND NOT "${shortname}" STREQUAL "OpenGLFunctions")
        set(module_header_content "${module_header_content}\n#include <Qt${shortname}/Qt${shortname}>")
    endif()
    set(post_header "${CMAKE_CURRENT_BINARY_DIR}/Qt${shortname}/Qt${shortname}_global.post.h")
    if(EXISTS "${post_header}")
        file(READ "${post_header}" post_header_content)
        set(module_header_content "${module_header_content}\n${post_header_content}")
    endif()
    file(WRITE ${module_header} "${module_header_content}")
endforeach()

# install
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/__init__.py"
        DESTINATION "${PYTHON_SITE_PACKAGES}/${BINDING_NAME}${pyside2_SUFFIX}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/_config.py"
        DESTINATION "${PYTHON_SITE_PACKAGES}/${BINDING_NAME}${pyside2_SUFFIX}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/_git_pyside_version.py"
        DESTINATION "${PYTHON_SITE_PACKAGES}/${BINDING_NAME}${pyside2_SUFFIX}")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/templates/core_common.xml
        DESTINATION share/PySide2${pyside_SUFFIX}/typesystems)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/templates/gui_common.xml
        DESTINATION share/PySide2${pyside_SUFFIX}/typesystems)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/templates/widgets_common.xml
        DESTINATION share/PySide2${pyside_SUFFIX}/typesystems)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/templates/datavisualization_common.xml
        DESTINATION share/PySide2${pyside_SUFFIX}/typesystems)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/templates/opengl_common.xml
        DESTINATION share/PySide2${pyside_SUFFIX}/typesystems)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/templates/openglfunctions_common.xml
        DESTINATION share/PySide2${pyside_SUFFIX}/typesystems)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/templates/webkitwidgets_common.xml
        DESTINATION share/PySide2${pyside_SUFFIX}/typesystems)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/templates/xml_common.xml
        DESTINATION share/PySide2${pyside_SUFFIX}/typesystems)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pyside2_global.h
        DESTINATION include/${BINDING_NAME}${pyside2_SUFFIX})
