project(pyside)

include(PySideModules)

find_package(Qt5Core)
find_package(Qt5Concurrent) # new in Qt5, from QtCore
find_package(Qt5Gui)        # reduced, small version without widgets
find_package(Qt5Widgets)    # widgets in their separate module
find_package(Qt5PrintSupport)
find_package(Qt5Xml)
find_package(Qt5XmlPatterns)
find_package(Qt5Svg)
find_package(Qt5Sql)
find_package(Qt5Designer)
find_package(Qt5UiTools)
find_package(Qt5Test)
find_package(Qt5Network)
find_package(Qt5WebKit)
find_package(Qt5WebKitWidgets)
find_package(Qt5OpenGL)
find_package(Qt5Script)
find_package(Qt5ScriptTools)
find_package(Qt5Help)
find_package(Qt5Declarative)
find_package(Qt5Multimedia)

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

#####
# This macro was needed to skip classes which did not build, yet.
# It replaces shiboken by a script that filters the missing classes
# away of the xxx_module_wrapper.cpp file.
# You can use it like so:
#
# skip_missing_classes(QtWidgets_SRC)
# create_pyside_module(QtWidgets
#                      ...
#

macro(skip_missing_classes sources)
    set(prev_shiboken_binary ${SHIBOKEN_BINARY})
    string(REPLACE ";" " " _escaped_sources "${${sources}}")
    file(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/fake_shiboken.sh"
         "
    echo \"$*\"
    ${prev_shiboken_binary} $*
    python3 ${CMAKE_CURRENT_BINARY_DIR}/filter_init.py ${_escaped_sources}
         ")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/filter_init.py"
         "if True: # allow the indentation
    import sys, os, re, pprint
    pprint.pprint(sys.argv)
    nset = set()
    for fname in sys.argv[1:]:
        name = os.path.splitext(os.path.basename(fname))[0]
        print(name)
        if name.endswith('module_wrapper'):
            fn = fname
        else:
            name = name.split('_wrapper')
            assert name[1] == ''
            nset.add(name[0])
    print(fn)
    with open(fn) as f:
        lines = f.readlines()
    removals = set()
    for idx, line in enumerate(lines):
        res = re.search(' init_(\\w+)', line)
        if res and res.group(1).lower() not in nset:
            removals.add(res.group(1))
            lines[idx] = '//' + line
    with open(fn, 'w') as f:
        f.writelines(lines)
    removals = sorted(list(removals))
    print('Removals:', removals)
    ")
    file(COPY ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/fake_shiboken.sh
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
      GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    set(SHIBOKEN_BINARY "${CMAKE_CURRENT_BINARY_DIR}/fake_shiboken.sh")
endmacro()

CHECK_PACKAGE_FOUND(Qt5Core)
CHECK_PACKAGE_FOUND(Qt5Concurrent)
CHECK_PACKAGE_FOUND(Qt5Gui)
CHECK_PACKAGE_FOUND(Qt5Widgets)
CHECK_PACKAGE_FOUND(Qt5PrintSupport)
CHECK_PACKAGE_FOUND(Qt5Xml)
CHECK_PACKAGE_FOUND(Qt5XmlPatterns)
CHECK_PACKAGE_FOUND(Qt5Svg)
CHECK_PACKAGE_FOUND(Qt5Sql)
CHECK_PACKAGE_FOUND(Qt5Designer)
CHECK_PACKAGE_FOUND(Qt5UiTools)
CHECK_PACKAGE_FOUND(Qt5Test)
CHECK_PACKAGE_FOUND(Qt5Network)
CHECK_PACKAGE_FOUND(Qt5WebKit)
CHECK_PACKAGE_FOUND(Qt5WebKitWidgets)
CHECK_PACKAGE_FOUND(Qt5Script)
CHECK_PACKAGE_FOUND(Qt5ScriptTools)
CHECK_PACKAGE_FOUND(Qt5Help)
CHECK_PACKAGE_FOUND(Qt5Multimedia)

# note: the order of this list is relevant for dependencies.
# For instance: Qt5Printsupport must come before Qt5WebKitWidgets
HAS_QT_MODULE(Qt5Core_FOUND QtCore)
HAS_QT_MODULE(Qt5Gui_FOUND QtGui)
HAS_QT_MODULE(Qt5Widgets_FOUND QtWidgets)
HAS_QT_MODULE(Qt5PrintSupport_FOUND QtPrintSupport)
HAS_QT_MODULE(Qt5Svg_FOUND QtSvg)
HAS_QT_MODULE(Qt5Sql_FOUND QtSql)
HAS_QT_MODULE(Qt5Network_FOUND QtNetwork)
HAS_QT_MODULE(Qt5Xml_FOUND QtXml)
HAS_QT_MODULE(Qt5XmlPatterns_FOUND QtXmlPatterns)
HAS_QT_MODULE(Qt5Test_FOUND QtTest)
HAS_QT_MODULE(Qt5WebKit_FOUND QtWebKit)
HAS_QT_MODULE(Qt5WebKitWidgets_FOUND QtWebKitWidgets)
###
# This module is deprecated! It should be replaces by QtGui.
# We keep the source code until we are done with QtGui migration.
# See http://doc.qt.io/qt-5/qtgui-index.html
# HAS_QT_MODULE(Qt5OpenGL_FOUND QtOpenGL)
###

if(Qt5Designer_FOUND)
    HAS_QT_MODULE(Qt5UiTools_FOUND QtUiTools)
endif()
HAS_QT_MODULE(Qt5Script_FOUND QtScript)
HAS_QT_MODULE(Qt5ScriptTools_FOUND QtScriptTools)
HAS_QT_MODULE(Qt5Declarative_FOUND QtDeclarative)
HAS_QT_MODULE(Qt5Help_FOUND QtHelp)
if(0)
  ### crashes shiboken, as soon as 'QAbstractVideoBuffer' is touched!
  HAS_QT_MODULE(Qt5Multimedia_FOUND QtMultimedia)
endif()

# install
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/__init__.py"
        DESTINATION "${SITE_PACKAGE}/${BINDING_NAME}${pyside_SUFFIX}")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/_utils.py
        DESTINATION "${SITE_PACKAGE}/${BINDING_NAME}${pyside_SUFFIX}")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/typesystem_templates.xml
        DESTINATION share/PySide${pyside_SUFFIX}/typesystems)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pyside_global.h
        DESTINATION include/${BINDING_NAME}${pyside_SUFFIX})

