project(QtScriptTools)

set(QtScriptTools_SRC
${QtScriptTools_GEN_DIR}/qscriptenginedebugger_wrapper.cpp
# module is always needed
${QtScriptTools_GEN_DIR}/qtscripttools_module_wrapper.cpp
)

set(QtScriptTools_include_dirs  ${QtScriptTools_SOURCE_DIR}
                                ${QtScriptTools_BINARY_DIR}
                                ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                                ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                                ${Qt${QT_MAJOR_VERSION}Widgets_INCLUDE_DIRS}
                                ${Qt${QT_MAJOR_VERSION}Script_INCLUDE_DIRS}
                                ${Qt${QT_MAJOR_VERSION}ScriptTools_INCLUDE_DIRS}
                                ${libpyside_SOURCE_DIR}
                                ${QtCore_GEN_DIR}
                                ${QtGui_GEN_DIR}
                                ${QtWidgets_GEN_DIR}
                                ${QtScript_GEN_DIR}
                                )

set(QtScriptTools_libraries pyside2
                            ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                            ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES}
                            ${Qt${QT_MAJOR_VERSION}Widgets_LIBRARIES}
                            ${Qt${QT_MAJOR_VERSION}Script_LIBRARIES}
                            ${Qt${QT_MAJOR_VERSION}ScriptTools_LIBRARIES})

set(QtScriptTools_deps QtCore QtScript QtGui QtWidgets)

create_pyside_module(NAME QtScriptTools
                     INCLUDE_DIRS QtScriptTools_include_dirs
                     LIBRARIES QtScriptTools_libraries
                     DEPS QtScriptTools_deps
                     TYPESYSTEM_PATH QtScriptTools_SOURCE_DIR
                     SOURCES QtScriptTools_SRC)
