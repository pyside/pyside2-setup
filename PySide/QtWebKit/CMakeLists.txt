project(QtWebKit)

set(QtWebKit_SRC
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebdatabase_wrapper.cpp
### use of undeclared identifier 'SBK_QWEBFRAME_IDX' ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebelement_wrapper.cpp
### we probably need to create 'webkitwidgets' for this to work!
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebelementcollection_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebhistory_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebhistoryinterface_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebhistoryitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebpluginfactory_mimetype_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebpluginfactory_plugin_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebpluginfactory_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebsecurityorigin_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebsettings_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/webcore_wrapper.cpp

# this file is always needed
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qtwebkit_module_wrapper.cpp
)

set(prev_shiboken_binary ${SHIBOKEN_BINARY})
string(REPLACE ";" " " QtWebKit_SRC_escape "${QtWebKit_SRC}")
file(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/fake_shiboken.sh"
     "
echo \"$*\"
${prev_shiboken_binary} $*
python3 ${CMAKE_CURRENT_BINARY_DIR}/filter_init.py ${QtWebKit_SRC_escape}
     ")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/filter_init.py"
     "#! env python
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

# fake these variables for qt5
set(QtWebKit_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(QtWebKit_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit)

make_path(QtWebKit_typesystem_path ${QtCore_SOURCE_DIR} ${QtGui_SOURCE_DIR} ${QtWidgets_SOURCE_DIR} ${QtNetwork_SOURCE_DIR}
                                   ${QtCore_BINARY_DIR} ${QtGui_BINARY_DIR} ${QtWidgets_BINARY_DIR} ${QtNetwork_BINARY_DIR}
                                   ${QtWebKit_SOURCE_DIR})

set(QtWebKit_include_dirs   ${CMAKE_CURRENT_SOURCE_DIR}
                            ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/
                            ${Qt5Core_INCLUDE_DIRS}
                            ${Qt5Gui_INCLUDE_DIRS}
                            ${Qt5Widgets_INCLUDE_DIRS}
                            ${Qt5Network_INCLUDE_DIRS}
                            ${Qt5WebKit_INCLUDE_DIRS}
                            ${SHIBOKEN_INCLUDE_DIR}
                            ${libpyside_SOURCE_DIR}
                            ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                            ${QtCore_BINARY_DIR}/PySide/QtCore/
                            ${QtGui_BINARY_DIR}/PySide/QtGui/
                            ${QtWidgets_BINARY_DIR}/PySide/QtWidgets/
                            ${QtNetwork_BINARY_DIR}/PySide/QtNetwork/ )
set(QtWebKit_libraries      pyside
                            ${SHIBOKEN_PYTHON_LIBRARIES}
                            ${SHIBOKEN_LIBRARY}
                            ${Qt5Webkit_LIBRARIES}
                            ${Qt5Network_LIBRARIES}
                            ${Qt5Widgets_LIBRARIES}
                            ${Qt5Gui_LIBRARIES}
                            ${Qt5Core_LIBRARIES}
                            )
set(QtWebKit_deps QtWidgets QtNetwork)
create_pyside_module(QtWebKit
                     QtWebKit_include_dirs
                     QtWebKit_libraries
                     QtWebKit_deps
                     QtWebKit_typesystem_path
                     QtWebKit_SRC
                     "")

