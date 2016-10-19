project(QtQuick)

set(QtQuick_registerType "${QtQuick_SOURCE_DIR}/pysidequickregistertype.cpp")

set(QtQuick_SRC
${QtQuick_GEN_DIR}/qquickframebufferobject_wrapper.cpp
${QtQuick_GEN_DIR}/qquickframebufferobject_renderer_wrapper.cpp
#${QtQuick_GEN_DIR}/qquicktexturefactory_wrapper.cpp
${QtQuick_GEN_DIR}/qquickimageprovider_wrapper.cpp
#${QtQuick_GEN_DIR}/qquicktransform_wrapper.cpp
${QtQuick_GEN_DIR}/qquickitem_wrapper.cpp
#${QtQuick_GEN_DIR}/qquickitemgrabresult_wrapper.cpp
${QtQuick_GEN_DIR}/qquickpainteditem_wrapper.cpp
${QtQuick_GEN_DIR}/qquickrendercontrol_wrapper.cpp
${QtQuick_GEN_DIR}/qquicktextdocument_wrapper.cpp
${QtQuick_GEN_DIR}/qquickview_wrapper.cpp
${QtQuick_GEN_DIR}/qquickwindow_wrapper.cpp
# module is always needed
${QtQuick_GEN_DIR}/qtquick_module_wrapper.cpp
)

make_path(QtQuick_typesystem_path ${QtCore_SOURCE_DIR} ${QtCore_BINARY_DIR}
                                  ${QtGui_SOURCE_DIR} ${QtGui_BINARY_DIR}
                                  ${QtNetwork_SOURCE_DIR} ${QtNetwork_BINARY_DIR}
                                  ${QtQml_SOURCE_DIR} ${QtQml_BINARY_DIR}
                                  ${QtQuick_SOURCE_DIR})

set(QtQuick_include_dirs  ${QtQuick_SOURCE_DIR}
                          ${Qt5Core_INCLUDE_DIRS}
                          ${Qt5Gui_INCLUDE_DIRS}
                          ${Qt5Network_INCLUDE_DIRS}
                          ${Qt5Qml_INCLUDE_DIRS}
                          ${Qt5Quick_INCLUDE_DIRS}
                          ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                          ${SHIBOKEN_INCLUDE_DIR}
                          ${libpyside_SOURCE_DIR}
                          ${QtGui_GEN_DIR}
                          ${QtCore_GEN_DIR}
                          ${QtNetwork_GEN_DIR}
                          ${QtQml_GEN_DIR}
                          ${QtQuick_GEN_DIR})

set(QtQuick_libraries pyside2
                      ${SHIBOKEN_PYTHON_LIBRARIES}
                      ${Qt5Core_LIBRARIES}
                      ${Qt5Gui_LIBRARIES}
                      ${Qt5Network_LIBRARIES}
                      ${Qt5Qml_LIBRARIES}
                      ${Qt5Quick_LIBRARIES})

set(QtQuick_deps QtGui QtNetwork QtQml)

create_pyside_module(QtQuick
                     QtQuick_include_dirs
                     QtQuick_libraries
                     QtQuick_deps
                     QtQuick_typesystem_path
                     QtQuick_SRC
                     QtQuick_registerType)
