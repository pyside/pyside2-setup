project(QtWidgets)


set(QtWidgets_OPTIONAL_SRC )
set(QtWidgets_DROPPED_ENTRIES )
## gone check_qt_class(Qt5Widgets QAbstractPageSetupDialog   QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QAbstractPrintDialog       QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QGtkStyle                  QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPageSetupDialog           QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPrintDialog               QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPrintEngine               QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPrintPreviewDialog        QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPrintPreviewWidget        QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPrinter                   QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QPrinterInfo               QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QSessionManager            QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QSizeGrip                  QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QSystemTrayIcon            QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)
check_qt_class(Qt5Widgets QMacStyle                  QtWidgets_OPTIONAL_SRC QtWidgets_DROPPED_ENTRIES)

set(QtWidgets_SRC
## add the files which are generated, later.
${SPECIFIC_OS_FILES}
# this file is always needed
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/qtwidgets_module_wrapper.cpp
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/typesystem_widgets.xml.in"
               "${CMAKE_CURRENT_BINARY_DIR}/typesystem_widgets.xml" @ONLY)

# fake these variables for qt5
set(QtWidgets_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(QtWidgets_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets)

set(prev_shiboken_binary ${SHIBOKEN_BINARY})
string(REPLACE ";" " " QtWidgets_SRC_escape "${QtWidgets_SRC}")
file(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/fake_shiboken.sh"
     "
echo \"$*\"
${prev_shiboken_binary} $*
python3 ${CMAKE_CURRENT_BINARY_DIR}/filter_init.py ${QtCore_SRC_escape}
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

set(QtWidgets_typesystem_path "${QtCore_SOURCE_DIR}${PATH_SEP}${QtCore_BINARY_DIR}${PATH_SEP}${QtGui_SOURCE_DIR}${PATH_SEP}${QtGui_BINARY_DIR}${PATH_SEP}${QtWidgets_SOURCE_DIR}")

set(QtWidgets_include_dirs  ${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWidgets/
                        ${pyside_SOURCE_DIR}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${Qt5Gui_INCLUDE_DIRS}
                        ${Qt5Widgets_INCLUDE_DIRS}
                        ${SHIBOKEN_INCLUDE_DIR}
                        ${libpyside_SOURCE_DIR}
                        ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                        ${QtCore_BINARY_DIR}/PySide/QtCore/
                        ${QtGui_BINARY_DIR}/PySide/QtGui/
                        )
set(QtWidgets_libraries   pyside
                      ${SHIBOKEN_PYTHON_LIBRARIES}
                      ${SHIBOKEN_LIBRARY}
                      ${Qt5Core_LIBRARIES}
                      ${Qt5Gui_LIBRARIES}
                      )
set(QtWidgets_deps "QtGui")

create_pyside_module(QtWidgets
                     QtWidgets_include_dirs
                     QtWidgets_libraries
                     QtWidgets_deps
                     QtWidgets_typesystem_path
                     QtWidgets_SRC
                     ""
                     ${CMAKE_CURRENT_BINARY_DIR}/typesystem_widgets.xml)
