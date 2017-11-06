project(QtTest)

set(QtTest_SRC
${QtTest_GEN_DIR}/qtest_pysideqtoucheventsequence_wrapper.cpp
${QtTest_GEN_DIR}/qtest_wrapper.cpp
# module is always needed
${QtTest_GEN_DIR}/qttest_module_wrapper.cpp
)

set(QtTest_include_dirs ${QtTest_SOURCE_DIR}
                        ${QtTest_BINARY_DIR}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${Qt5Gui_INCLUDE_DIRS}
                        ${Qt5Widgets_INCLUDE_DIRS}
                        ${Qt5Test_INCLUDE_DIRS}
                        ${SHIBOKEN_INCLUDE_DIR}
                        ${libpyside_SOURCE_DIR}
                        ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                        ${QtCore_GEN_DIR}
                        ${QtGui_GEN_DIR}
                        ${QtWidgets_GEN_DIR}
                        )
set(QtTest_libraries    pyside2
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
                     QtTest_SOURCE_DIR
                     QtTest_SRC
                     "")
