project(QtTest)

set(QtTest_SRC
${QtTest_GEN_DIR}/qtest_pysideqtoucheventsequence_wrapper.cpp
${QtTest_GEN_DIR}/qtest_wrapper.cpp
# module is always needed
${QtTest_GEN_DIR}/qttest_module_wrapper.cpp
)

configure_file("${QtTest_SOURCE_DIR}/QtTest_global.pre.h.in"
               "${QtTest_BINARY_DIR}/QtTest_global.pre.h" @ONLY)

configure_file("${QtTest_SOURCE_DIR}/QtTest_global.post.h.in"
               "${QtTest_BINARY_DIR}/QtTest_global.post.h" @ONLY)

set(QtTest_include_dirs ${QtTest_SOURCE_DIR}
                        ${QtTest_BINARY_DIR}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${Qt5Gui_INCLUDE_DIRS}
                        ${Qt5Widgets_INCLUDE_DIRS}
                        ${Qt5Test_INCLUDE_DIRS}
                        ${libpyside_SOURCE_DIR}
                        ${QtCore_GEN_DIR}
                        ${QtGui_GEN_DIR}
                        ${QtWidgets_GEN_DIR}
                        )
set(QtTest_libraries    pyside2
                        ${Qt5Test_LIBRARIES}
                        ${Qt5Widgets_LIBRARIES}
                        ${Qt5Gui_LIBRARIES}
                        ${Qt5Core_LIBRARIES}
                        )
set(QtTest_deps  QtWidgets)

create_pyside_module(NAME QtTest
                     INCLUDE_DIRS QtTest_include_dirs
                     LIBRARIES QtTest_libraries
                     DEPS QtTest_deps
                     TYPESYSTEM_PATH QtTest_SOURCE_DIR
                     SOURCES QtTest_SRC)
