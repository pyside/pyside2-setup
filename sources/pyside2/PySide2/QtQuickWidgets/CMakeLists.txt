project(QtQuickWidgets)

set(QtQuickWidgets_SRC
${QtQuickWidgets_GEN_DIR}/qquickwidget_wrapper.cpp
# module is always needed
${QtQuickWidgets_GEN_DIR}/qtquickwidgets_module_wrapper.cpp
)

set(QtQuickWidgets_include_dirs  ${QtQuickWidgets_SOURCE_DIR}
                                 ${QtQml_SOURCE_DIR}
                                 ${Qt5Core_INCLUDE_DIRS}
                                 ${Qt5Gui_INCLUDE_DIRS}
                                 ${Qt5Widgets_INCLUDE_DIRS}
                                 ${Qt5Network_INCLUDE_DIRS}
                                 ${Qt5Quick_INCLUDE_DIRS}
                                 ${Qt5Qml_INCLUDE_DIRS}
                                 ${Qt5QuickWidgets_INCLUDE_DIRS}
                                 ${libpyside_SOURCE_DIR}
                                 ${QtGui_GEN_DIR}
                                 ${QtCore_GEN_DIR}
                                 ${QtWidgets_GEN_DIR}
                                 ${QtNetwork_GEN_DIR}
                                 ${QtQuick_GEN_DIR}
                                 ${QtQml_GEN_DIR}
                                 ${QtQuickWidgets_GEN_DIR})

set(QtQuickWidgets_libraries pyside2
                             ${Qt5Core_LIBRARIES}
                             ${Qt5Gui_LIBRARIES}
                             ${Qt5Network_LIBRARIES}
                             ${Qt5Widgets_LIBRARIES}
                             ${Qt5Quick_LIBRARIES}
                             ${Qt5Qml_LIBRARIES}
                             ${Qt5QuickWidgets_LIBRARIES})

set(QtQuickWidgets_deps QtGui QtQml QtQuick QtWidgets QtNetwork)

create_pyside_module(NAME QtQuickWidgets
                     INCLUDE_DIRS QtQuickWidgets_include_dirs
                     LIBRARIES QtQuickWidgets_libraries
                     DEPS QtQuickWidgets_deps
                     TYPESYSTEM_PATH QtQuickWidgets_SOURCE_DIR
                     SOURCES QtQuickWidgets_SRC)
