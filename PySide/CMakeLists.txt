project(pyside)

include(PySideModules)

# Configure include based on platform
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/global.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/pyside_global.h" @ONLY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
               "${CMAKE_CURRENT_BINARY_DIR}/__init__.py" @ONLY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/_utils.py.in"
               "${CMAKE_CURRENT_BINARY_DIR}/_utils.py" @ONLY)

macro(CHECK_VAR name)
    if(${name})
        message("module ${name} found")
    else()
        message(FATAL_ERROR "${name} MISSING")
    endif()
endmacro()

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

CHECK_VAR(Qt5Core_FOUND)
CHECK_VAR(Qt5Concurrent_FOUND)
CHECK_VAR(Qt5Gui_FOUND)
CHECK_VAR(Qt5Widgets_FOUND)
CHECK_VAR(Qt5Xml_FOUND)
CHECK_VAR(Qt5XmlPatterns_FOUND)
CHECK_VAR(Qt5Svg_FOUND)
CHECK_VAR(Qt5Sql_FOUND)
CHECK_VAR(Qt5Designer_FOUND)
CHECK_VAR(Qt5UiTools_FOUND)
CHECK_VAR(Qt5Test_FOUND)
CHECK_VAR(Qt5Network_FOUND)

HAS_QT_MODULE(Qt5Core_FOUND QtCore)
HAS_QT_MODULE(Qt5Gui_FOUND QtGui)
### depends on QtGui HAS_QT_MODULE(Qt5Sql_FOUND QtSql)
# compiles, skip   HAS_QT_MODULE(Qt5Network_FOUND QtNetwork)
# need to split this HAS_QT_MODULE(Qt5Gui_FOUND QtGui)
# compiles, skip   HAS_QT_MODULE(Qt5Xml_FOUND QtXml)
### generates no classes HAS_QT_MODULE(Qt5XmlPatterns_FOUND QtXmlPatterns)
### depends on QtGui HAS_QT_MODULE(Qt5Test_FOUND QtTest)
### depends on QtGui HAS_QT_MODULE(Qt5Svg_FOUND QtSvg)

HAS_QT_MODULE(QT_QTWEBKIT_FOUND QtWebKit)
HAS_QT_MODULE(QT_QTOPENGL_FOUND QtOpenGL)
if(Qt5Designer_FOUND)
    ### depends on QtGui HAS_QT_MODULE(Qt5UiTools_FOUND QtUiTools)
endif()
HAS_QT_MODULE(QT_QTHELP_FOUND QtHelp)
HAS_QT_MODULE(QT_QTSCRIPT_FOUND QtScript)
HAS_QT_MODULE(QT_QTSCRIPTTOOLS_FOUND QtScriptTools)
HAS_QT_MODULE(QT_QTMULTIMEDIA_FOUND QtMultimedia)
HAS_QT_MODULE(QT_QTDECLARATIVE_FOUND QtDeclarative)

# install
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/__init__.py"
        DESTINATION "${SITE_PACKAGE}/${BINDING_NAME}${pyside_SUFFIX}")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/_utils.py
        DESTINATION "${SITE_PACKAGE}/${BINDING_NAME}${pyside_SUFFIX}")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/typesystem_templates.xml
        DESTINATION share/PySide${pyside_SUFFIX}/typesystems)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pyside_global.h
        DESTINATION include/${BINDING_NAME}${pyside_SUFFIX})

