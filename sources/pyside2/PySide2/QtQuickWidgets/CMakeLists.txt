project(QtQuickWidgets)

set(QtQuickWidgets_SRC
${QtQuickWidgets_GEN_DIR}/qquickwidget_wrapper.cpp
# module is always needed
${QtQuickWidgets_GEN_DIR}/qtquickwidgets_module_wrapper.cpp
)

set(QtQuickWidgets_include_dirs  ${QtQuickWidgets_SOURCE_DIR}
                                 ${QtQml_SOURCE_DIR}
                                 ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                                 ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                                 ${Qt${QT_MAJOR_VERSION}Widgets_INCLUDE_DIRS}
                                 ${Qt${QT_MAJOR_VERSION}Network_INCLUDE_DIRS}
                                 ${Qt${QT_MAJOR_VERSION}Quick_INCLUDE_DIRS}
                                 ${Qt${QT_MAJOR_VERSION}Qml_INCLUDE_DIRS}
                                 ${Qt${QT_MAJOR_VERSION}QuickWidgets_INCLUDE_DIRS}
                                 ${libpyside_SOURCE_DIR}
                                 ${QtGui_GEN_DIR}
                                 ${QtCore_GEN_DIR}
                                 ${QtWidgets_GEN_DIR}
                                 ${QtNetwork_GEN_DIR}
                                 ${QtQuick_GEN_DIR}
                                 ${QtQml_GEN_DIR}
                                 ${QtQuickWidgets_GEN_DIR})

set(QtQuickWidgets_libraries pyside2
                             ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                             ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES}
                             ${Qt${QT_MAJOR_VERSION}Network_LIBRARIES}
                             ${Qt${QT_MAJOR_VERSION}Widgets_LIBRARIES}
                             ${Qt${QT_MAJOR_VERSION}Quick_LIBRARIES}
                             ${Qt${QT_MAJOR_VERSION}Qml_LIBRARIES}
                             ${Qt${QT_MAJOR_VERSION}QuickWidgets_LIBRARIES})

set(QtQuickWidgets_deps QtGui QtQml QtQuick QtWidgets QtNetwork)

create_pyside_module(NAME QtQuickWidgets
                     INCLUDE_DIRS QtQuickWidgets_include_dirs
                     LIBRARIES QtQuickWidgets_libraries
                     DEPS QtQuickWidgets_deps
                     TYPESYSTEM_PATH QtQuickWidgets_SOURCE_DIR
                     SOURCES QtQuickWidgets_SRC)
