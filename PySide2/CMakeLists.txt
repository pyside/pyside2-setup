project(pyside2)

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
find_package(Qt5Multimedia)
find_package(Qt5MultimediaWidgets)
find_package(Qt5Qml)
find_package(Qt5Quick)
find_package(Qt5QuickWidgets)
find_package(Qt5WebChannel)
find_package(Qt5WebEngineWidgets)
find_package(Qt5WebSockets)

if(UNIX AND NOT APPLE)
  find_package(Qt5X11Extras) # new in Qt5, from QtGui
endif()

# Configure include based on platform
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/global.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/pyside2_global.h" @ONLY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
               "${CMAKE_CURRENT_BINARY_DIR}/__init__.py" @ONLY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/_utils.py.in"
               "${CMAKE_CURRENT_BINARY_DIR}/_utils.py" @ONLY)

macro(CHECK_PACKAGE_FOUND name)
    set(_name_found "${name}_FOUND")
    if(${_name_found})
        message("module ${name} found")

        # Hoist this to the parent scope to make sure all tests get built
        set("${name}_FOUND" 1 PARENT_SCOPE)
    else()
        if("${ARGN}" STREQUAL "opt")
            message(STATUS "optional module ${name} skipped")
        else()
            message(FATAL_ERROR "module ${name} MISSING")
        endif()
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
    # this line signals postprocessing to macro "create_pyside_module".
    set(${sources}_skipped_files 1)
    # you just need to pass exactly the same sources variable.
    string(REPLACE ";" "\n" _escaped_sources "${${sources}}")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/filter_init.py"
         "if True: # allow the indentation
    files = '''\n${_escaped_sources}\n'''.strip().split('\\n')
    import sys, os, re, pprint
    pprint.pprint(files)
    nset = set()
    for fname in files:
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
    fix_header_fname = '${CMAKE_CURRENT_SOURCE_DIR}/fixup_headers.py'
    if sys.platform == 'win32' and os.path.exists(fix_header_fname):
        global target_dir
        target_dir = '${${PROJECT_NAME}_GEN_DIR}'

        with open(fix_header_fname) as f:
            code = compile(f.read(), 'nebbich', 'exec')
        exec(code, globals(), locals())
    ")
endmacro()

CHECK_PACKAGE_FOUND(Qt5Core)
CHECK_PACKAGE_FOUND(Qt5Concurrent)
CHECK_PACKAGE_FOUND(Qt5Gui)
CHECK_PACKAGE_FOUND(Qt5Widgets)
CHECK_PACKAGE_FOUND(Qt5PrintSupport)
CHECK_PACKAGE_FOUND(Qt5Xml)
CHECK_PACKAGE_FOUND(Qt5XmlPatterns opt)
CHECK_PACKAGE_FOUND(Qt5Svg opt)
CHECK_PACKAGE_FOUND(Qt5PrintSupport)
CHECK_PACKAGE_FOUND(Qt5Sql)
CHECK_PACKAGE_FOUND(Qt5Designer opt)
CHECK_PACKAGE_FOUND(Qt5UiTools opt)
CHECK_PACKAGE_FOUND(Qt5Test)
CHECK_PACKAGE_FOUND(Qt5Network)
CHECK_PACKAGE_FOUND(Qt5WebKit opt)
CHECK_PACKAGE_FOUND(Qt5WebKitWidgets opt)
CHECK_PACKAGE_FOUND(Qt5Script opt)
CHECK_PACKAGE_FOUND(Qt5ScriptTools opt)
CHECK_PACKAGE_FOUND(Qt5Help opt)
CHECK_PACKAGE_FOUND(Qt5Multimedia opt)
CHECK_PACKAGE_FOUND(Qt5MultimediaWidgets opt)
CHECK_PACKAGE_FOUND(Qt5Qml opt)
CHECK_PACKAGE_FOUND(Qt5Quick opt)
CHECK_PACKAGE_FOUND(Qt5QuickWidgets opt)
CHECK_PACKAGE_FOUND(Qt5WebChannel opt)
CHECK_PACKAGE_FOUND(Qt5WebEngineWidgets opt)
CHECK_PACKAGE_FOUND(Qt5WebSockets opt)

if(UNIX AND NOT APPLE)
  CHECK_PACKAGE_FOUND(Qt5X11Extras opt)
endif()

# note: the order of this list is relevant for dependencies.
# For instance: Qt5Printsupport must come before Qt5WebKitWidgets
HAS_QT_MODULE(Qt5Core_FOUND QtCore)
if(NOT MSVC)
    # right now this does not build on windows
    HAS_QT_MODULE(Qt5Concurrent_FOUND QtConcurrent)
else()
    # Note: in order to use this variable in a sibling project (tests),
    # we need to either make the value persistent like so:
    # set(DISABLE_QtConcurrent 1 CACHE INTERNAL "")
    # or export it to the caller's scope by the PARENT_SCOPE specifier.
    # This is preferable, because there are no sticky side effects.
    set(DISABLE_QtConcurrent 1 PARENT_SCOPE)
ENDIF()
HAS_QT_MODULE(Qt5Gui_FOUND QtGui)
HAS_QT_MODULE(Qt5Widgets_FOUND QtWidgets)
# This module is deprecated and should be replaces by QtGui.
# See http://doc.qt.io/qt-5/qtgui-index.html
# Update: We support QtOpenGL now!
# Note: The dependency is wrong: QtWidgets is needed!
HAS_QT_MODULE(Qt5OpenGL_FOUND QtOpenGL)
HAS_QT_MODULE(Qt5PrintSupport_FOUND QtPrintSupport)
HAS_QT_MODULE(Qt5Svg_FOUND QtSvg)
HAS_QT_MODULE(Qt5Sql_FOUND QtSql)
HAS_QT_MODULE(Qt5Network_FOUND QtNetwork)
HAS_QT_MODULE(Qt5Xml_FOUND QtXml)
HAS_QT_MODULE(Qt5XmlPatterns_FOUND QtXmlPatterns)
HAS_QT_MODULE(Qt5Test_FOUND QtTest)
HAS_QT_MODULE(Qt5WebKit_FOUND QtWebKit)
if(NOT MSVC)
    # right now this does not build on windows
    HAS_QT_MODULE(Qt5WebKitWidgets_FOUND QtWebKitWidgets)
else()
    set(DISABLE_QtWebKitWidgets 1 PARENT_SCOPE)
ENDIF()

if(Qt5Designer_FOUND)
    HAS_QT_MODULE(Qt5UiTools_FOUND QtUiTools)
else()
    set(DISABLE_QtUiTools 1 PARENT_SCOPE)
endif()
HAS_QT_MODULE(Qt5Script_FOUND QtScript)
if(NOT MSVC)
    # right now this does not build on windows
    HAS_QT_MODULE(Qt5ScriptTools_FOUND QtScriptTools)
else()
    set(DISABLE_QtScriptTools 1 PARENT_SCOPE)
ENDIF()
HAS_QT_MODULE(Qt5Help_FOUND QtHelp)
HAS_QT_MODULE(Qt5Multimedia_FOUND QtMultimedia)
HAS_QT_MODULE(Qt5MultimediaWidgets_FOUND QtMultimediaWidgets)
HAS_QT_MODULE(Qt5Qml_FOUND QtQml)
HAS_QT_MODULE(Qt5Quick_FOUND QtQuick)
HAS_QT_MODULE(Qt5QuickWidgets_FOUND QtQuickWidgets)
HAS_QT_MODULE(Qt5WebChannel_FOUND QtWebChannel)
HAS_QT_MODULE(Qt5WebEngineWidgets_FOUND QtWebEngineWidgets)
HAS_QT_MODULE(Qt5WebSockets_FOUND QtWebSockets)

if(UNIX AND NOT APPLE)
    has_qt_module(Qt5X11Extras_FOUND QtX11Extras)
endif()

# install
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/__init__.py"
        DESTINATION "${SITE_PACKAGE}/${BINDING_NAME}${pyside2_SUFFIX}")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/_utils.py
        DESTINATION "${SITE_PACKAGE}/${BINDING_NAME}${pyside2_SUFFIX}")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/typesystem_templates.xml
        DESTINATION share/PySide2${pyside_SUFFIX}/typesystems)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pyside2_global.h
        DESTINATION include/${BINDING_NAME}${pyside2_SUFFIX})

