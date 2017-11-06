project(QtScriptTools)

set(QtScriptTools_SRC
${QtScriptTools_GEN_DIR}/qscriptenginedebugger_wrapper.cpp
# module is always needed
${QtScriptTools_GEN_DIR}/qtscripttools_module_wrapper.cpp
)

set(QtScriptTools_include_dirs  ${QtScriptTools_SOURCE_DIR}
                                ${QtScriptTools_BINARY_DIR}
                                ${Qt5Core_INCLUDE_DIRS}
                                ${Qt5Gui_INCLUDE_DIRS}
                                ${Qt5Widgets_INCLUDE_DIRS}
                                ${Qt5Script_INCLUDE_DIRS}
                                ${Qt5ScriptTools_INCLUDE_DIRS}
                                ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                                ${SHIBOKEN_INCLUDE_DIR}
                                ${libpyside_SOURCE_DIR}
                                ${QtCore_GEN_DIR}
                                ${QtGui_GEN_DIR}
                                ${QtWidgets_GEN_DIR}
                                ${QtScript_GEN_DIR}
                                )

set(QtScriptTools_libraries pyside2
                            ${SHIBOKEN_PYTHON_LIBRARIES}
                            ${Qt5Core_LIBRARIES}
                            ${Qt5Gui_LIBRARIES}
                            ${Qt5Widgets_LIBRARIES}
                            ${Qt5Script_LIBRARIES}
                            ${Qt5ScriptTools_LIBRARIES})

set(QtScriptTools_deps QtCore QtScript QtGui QtWidgets)

create_pyside_module(QtScriptTools
                     QtScriptTools_include_dirs
                     QtScriptTools_libraries
                     QtScriptTools_deps
                     QtScriptTools_SOURCE_DIR
                     QtScriptTools_SRC
                     "")
