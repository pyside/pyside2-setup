project(QtUiTools)

set(QtUiTools_SRC
${QtUiTools_GEN_DIR}/quiloader_wrapper.cpp
# module is always needed
${QtUiTools_GEN_DIR}/qtuitools_module_wrapper.cpp
)

set(QtUiTools_glue_sources
    "${QtUiTools_SOURCE_DIR}/glue/plugins.h"
)

set(QtUiTools_include_dirs  ${QtUiTools_SOURCE_DIR}
                            ${QtUiTools_BINARY_DIR}
                            ${Qt5Core_INCLUDE_DIRS}
                            ${Qt5Gui_INCLUDE_DIRS}
                            ${Qt5Widgets_INCLUDE_DIRS}
                            ${Qt5Xml_INCLUDE_DIRS}
                            ${Qt5Designer_INCLUDE_DIRS}
                            ${Qt5UiTools_INCLUDE_DIRS}
                            ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                            ${SHIBOKEN_INCLUDE_DIR}
                            ${libpyside_SOURCE_DIR}
                            ${plugins_SOURCE_DIR}
                            ${QtCore_GEN_DIR}
                            ${QtXml_GEN_DIR}
                            ${QtGui_GEN_DIR}
                            ${QtWidgets_GEN_DIR}
                            )
set(QtUiTools_libraries     pyside2
                            uiplugin
                            ${SHIBOKEN_PYTHON_LIBRARIES}
                            ${Qt5UiTools_LIBRARIES}
                            ${Qt5Core_LIBRARIES}
                            ${Qt5Gui_LIBRARIES}
                            ${Qt5Widgets_LIBRARIES}
                            )
set(QtUiTools_deps QtWidgets QtXml)
create_pyside_module(NAME QtUiTools
                     INCLUDE_DIRS QtUiTools_include_dirs
                     LIBRARIES QtUiTools_libraries
                     DEPS QtUiTools_deps
                     TYPESYSTEM_PATH QtUiTools_SOURCE_DIR
                     SOURCES QtUiTools_SRC
                     GLUE_SOURCES QtUiTools_glue_sources)
