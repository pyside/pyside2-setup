project(QtQuickControls2)

set(QtQuickControls2_SRC
${QtQuickControls2_GEN_DIR}/qquickstyle_wrapper.cpp
# module is always needed
${QtQuickControls2_GEN_DIR}/qtquickcontrols2_module_wrapper.cpp
)


set(QtQuickControls2_include_dirs ${QtQuickControls2_SOURCE_DIR}
                                  ${QtQml_SOURCE_DIR}
                                  ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                                  ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                                  ${Qt${QT_MAJOR_VERSION}Network_INCLUDE_DIRS}
                                  ${Qt${QT_MAJOR_VERSION}Qml_INCLUDE_DIRS}
                                  ${Qt${QT_MAJOR_VERSION}Quick_INCLUDE_DIRS}
                                  ${Qt${QT_MAJOR_VERSION}QuickControls2_INCLUDE_DIRS}
                                  ${libpyside_SOURCE_DIR}
                                  ${QtGui_GEN_DIR}
                                  ${QtCore_GEN_DIR}
                                  ${QtNetwork_GEN_DIR}
                                  ${QtQml_GEN_DIR}
                                  ${QtQuick_GEN_DIR}
                                  ${QtQuickControls2_GEN_DIR})

set(QtQuickControls2_libraries pyside2
                      ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                      ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES}
                      ${Qt${QT_MAJOR_VERSION}Network_LIBRARIES}
                      ${Qt${QT_MAJOR_VERSION}Qml_LIBRARIES}
                      ${Qt${QT_MAJOR_VERSION}Quick_LIBRARIES}
                      ${Qt${QT_MAJOR_VERSION}QuickControls2_LIBRARIES})

set(QtQuickControls2_deps QtGui QtNetwork QtQml QtQuick)

create_pyside_module(NAME QtQuickControls2
                     INCLUDE_DIRS QtQuickControls2_include_dirs
                     LIBRARIES QtQuickControls2_libraries
                     DEPS QtQuickControls2_deps
                     TYPESYSTEM_PATH QtQuickControls2_SOURCE_DIR
                     SOURCES QtQuickControls2_SRC)
