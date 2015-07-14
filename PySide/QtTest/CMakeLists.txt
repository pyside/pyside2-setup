project(QtTest)

set(QtTest_SRC
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtTest/qtest_pysideqtoucheventsequence_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtTest/qtest_wrapper.cpp

# this file is always needed
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtTest/qttest_module_wrapper.cpp
)

# fake these variables for qt5
set(QtTest_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(QtTest_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtTest)

make_path(QtTest_typesystem_path ${QtCore_SOURCE_DIR} ${QtGui_SOURCE_DIR} ${QtWidgets_SOURCE_DIR}
                                 ${QtCore_BINARY_DIR} ${QtGui_BINARY_DIR} ${QtWidgets_BINARY_DIR}
                                 ${QtTest_SOURCE_DIR})

set(QtTest_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtTest/
                        ${Qt5Core_INCLUDE_DIRS}
                        ${Qt5Gui_INCLUDE_DIRS}
                        ${Qt5Widgets_INCLUDE_DIRS}
                        ${Qt5Test_INCLUDE_DIRS}
                        ${SHIBOKEN_INCLUDE_DIR}
                        ${libpyside_SOURCE_DIR}
                        ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                        ${QtCore_BINARY_DIR}/PySide/QtCore/
                        ${QtGui_BINARY_DIR}/PySide/QtGui/
                        ${QtWidgets_BINARY_DIR}/PySide/QtWidgets/
                        )
set(QtTest_libraries    pyside
                        ${SHIBOKEN_PYTHON_LIBRARIES}
                        ${SHIBOKEN_LIBRARY}
                        ${Qt5Test_LIBRARIES}
                        ${Qt5Widgets_LIBRARIES}
                        ${Qt5Gui_LIBRARIES}
                        ${Qt5Core_LIBRARIES}
                        )
set(QtTest_deps  QtWidgets)
create_pyside_module(QtTest
                     QtTest_include_dirs
                     QtTest_libraries
                     QtTest_deps
                     QtTest_typesystem_path
                     QtTest_SRC
                     "")
