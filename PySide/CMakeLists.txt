project(pyside)

include(PySideModules)

find_package(Qt5Core)
find_package(Qt5Concurrent) # new in Qt5, from QtCore
find_package(Qt5Gui)        # reduced, small version without widgets
find_package(Qt5Widgets)    # widgets in their separate module
find_package(Qt5Xml)
find_package(Qt5XmlPatterns)
find_package(Qt5Svg)
find_package(Qt5Sql)
find_package(Qt5Designer)
find_package(Qt5UiTools)
find_package(Qt5Test)
find_package(Qt5Network)
find_package(Qt5Webkit)
find_package(Qt5OpenGL)
find_package(Qt5Script)
find_package(Qt5ScriptTools)
find_package(Qt5Help)
find_package(Qt5Declarative)

# Configure include based on platform
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/global.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/pyside_global.h" @ONLY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
               "${CMAKE_CURRENT_BINARY_DIR}/__init__.py" @ONLY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/_utils.py.in"
               "${CMAKE_CURRENT_BINARY_DIR}/_utils.py" @ONLY)

macro(MAKE_PATH varname)
   # accepts any number of path variables
   string(REPLACE ";" "${PATH_SEP}" ${varname} "${ARGN}")
endmacro()

macro(CHECK_PACKAGE_FOUND name)
    set(_name_found "${name}_FOUND")
    if(${_name_found})
        message("module ${name} found")
    else()
        message(FATAL_ERROR "module ${name} MISSING")
    endif()
endmacro()

CHECK_PACKAGE_FOUND(Qt5Core)
CHECK_PACKAGE_FOUND(Qt5Concurrent)
CHECK_PACKAGE_FOUND(Qt5Gui)
CHECK_PACKAGE_FOUND(Qt5Widgets)
CHECK_PACKAGE_FOUND(Qt5Xml)
CHECK_PACKAGE_FOUND(Qt5XmlPatterns)
CHECK_PACKAGE_FOUND(Qt5Svg)
CHECK_PACKAGE_FOUND(Qt5Sql)
CHECK_PACKAGE_FOUND(Qt5Designer)
CHECK_PACKAGE_FOUND(Qt5UiTools)
CHECK_PACKAGE_FOUND(Qt5Test)
CHECK_PACKAGE_FOUND(Qt5Network)
CHECK_PACKAGE_FOUND(Qt5Webkit)
# Careful: Qt5Webkit_FOUND but Qt5WebKit_INCLUDE_DIRS
# XXX fix this in a better way, and report this to Qt!
CHECK_PACKAGE_FOUND(Qt5Script)
### ??? CHECK_PACKAGE_FOUND(Qt5ScriptTools)
### ??? CHECK_PACKAGE_FOUND(Qt5Help)
set(Qt5Help_FOUND 1)

HAS_QT_MODULE(Qt5Core_FOUND QtCore)
HAS_QT_MODULE(Qt5Gui_FOUND QtGui)
HAS_QT_MODULE(Qt5Widgets_FOUND QtWidgets)
HAS_QT_MODULE(Qt5Svg_FOUND QtSvg)
HAS_QT_MODULE(Qt5Sql_FOUND QtSql)
HAS_QT_MODULE(Qt5Network_FOUND QtNetwork)
HAS_QT_MODULE(Qt5Xml_FOUND QtXml)
HAS_QT_MODULE(Qt5XmlPatterns_FOUND QtXmlPatterns)
HAS_QT_MODULE(Qt5Test_FOUND QtTest)
HAS_QT_MODULE(Qt5Webkit_FOUND QtWebKit)

###
# This module is deprecated! It should be replaces by QtGui.
# We keep the source code until we are done with QtGui migration.
# See http://doc.qt.io/qt-5/qtgui-index.html
# HAS_QT_MODULE(Qt5OpenGL_FOUND QtOpenGL)
###

if(Qt5Designer_FOUND)
    HAS_QT_MODULE(Qt5UiTools_FOUND QtUiTools)
endif()
### ??? HAS_QT_MODULE(Qt5Help_FOUND QtHelp)
HAS_QT_MODULE(Qt5Script_FOUND QtScript)
HAS_QT_MODULE(Qt5ScriptTools_FOUND QtScriptTools)
HAS_QT_MODULE(QT_QTMULTIMEDIA_FOUND QtMultimedia)
HAS_QT_MODULE(Qt5Declarative_FOUND QtDeclarative)
HAS_QT_MODULE(Qt5Help_FOUND QtHelp)

# install
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/__init__.py"
        DESTINATION "${SITE_PACKAGE}/${BINDING_NAME}${pyside_SUFFIX}")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/_utils.py
        DESTINATION "${SITE_PACKAGE}/${BINDING_NAME}${pyside_SUFFIX}")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/typesystem_templates.xml
        DESTINATION share/PySide${pyside_SUFFIX}/typesystems)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pyside_global.h
        DESTINATION include/${BINDING_NAME}${pyside_SUFFIX})

