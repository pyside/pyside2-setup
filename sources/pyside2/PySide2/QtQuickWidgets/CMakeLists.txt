project(QtQuickWidgets)

set(QtQuickWidgets_SRC
${QtQuickWidgets_GEN_DIR}/qquickwidget_wrapper.cpp
# module is always needed
${QtQuickWidgets_GEN_DIR}/qtquickwidgets_module_wrapper.cpp
)

make_path(QtQuickWidgets_typesystem_path ${QtCore_SOURCE_DIR} ${QtGui_SOURCE_DIR} ${QtWidgets_SOURCE_DIR}
                                         ${QtCore_BINARY_DIR} ${QtGui_BINARY_DIR} ${QtWidgets_BINARY_DIR}
                                         ${QtNetwork_SOURCE_DIR} ${QtNetwork_BINARY_DIR} ${QtQuick_SOURCE_DIR}
                                         ${QtQuick_BINARY_DIR} ${QtQml_SOURCE_DIR} ${QtQml_BINARY_DIR}
                                         ${QtQuickWidgets_SOURCE_DIR})

set(QtQuickWidgets_include_dirs  ${QtQuickWidgets_SOURCE_DIR}
                                 ${QtQml_SOURCE_DIR}
                                 ${Qt5Core_INCLUDE_DIRS}
                                 ${Qt5Gui_INCLUDE_DIRS}
                                 ${Qt5Widgets_INCLUDE_DIRS}
                                 ${Qt5Network_INCLUDE_DIRS}
                                 ${Qt5Quick_INCLUDE_DIRS}
                                 ${Qt5Qml_INCLUDE_DIRS}
                                 ${Qt5QuickWidgets_INCLUDE_DIRS}
                                 ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                                 ${SHIBOKEN_INCLUDE_DIR}
                                 ${libpyside_SOURCE_DIR}
                                 ${QtGui_GEN_DIR}
                                 ${QtCore_GEN_DIR}
                                 ${QtWidgets_GEN_DIR}
                                 ${QtNetwork_GEN_DIR}
                                 ${QtQuick_GEN_DIR}
                                 ${QtQml_GEN_DIR}
                                 ${QtQuickWidgets_GEN_DIR})

set(QtQuickWidgets_libraries pyside2
                             ${SHIBOKEN_PYTHON_LIBRARIES}
                             ${Qt5Core_LIBRARIES}
                             ${Qt5Gui_LIBRARIES}
                             ${Qt5Network_LIBRARIES}
                             ${Qt5Widgets_LIBRARIES}
                             ${Qt5Quick_LIBRARIES}
                             ${Qt5Qml_LIBRARIES}
                             ${Qt5QuickWidgets_LIBRARIES})

set(QtQuickWidgets_deps QtGui QtQml QtQuick QtWidgets QtNetwork)

create_pyside_module(QtQuickWidgets
                     QtQuickWidgets_include_dirs
                     QtQuickWidgets_libraries
                     QtQuickWidgets_deps
                     QtQuickWidgets_typesystem_path
                     QtQuickWidgets_SRC
                     "")
