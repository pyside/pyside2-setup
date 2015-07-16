project(QtScriptTools)

set(QtScriptTools_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtScriptTools/qtscripttools_module_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtScriptTools/qscriptenginedebugger_wrapper.cpp
)

set(QtScriptTools_typesystem_path "${QtCore_SOURCE_DIR}${PATH_SEP}${QtCore_BINARY_DIR}${PATH_SEP}${QtGui_SOURCE_DIR}${PATH_SEP}${QtScript_SOURCE_DIR}${PATH_SEP}${QtScriptTools_SOURCE_DIR}${PATH_SEP}${QtGui_BINARY_DIR}")

set(QtScriptTools_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}
                                ${Qt5Core_INCLUDE_DIRS}
                                ${Qt5Gui_INCLUDE_DIRS}
                                ${Qt5Script_INCLUDE_DIRS}
                                ${Qt5ScriptTools_INCLUDE_DIRS}
                                ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                                ${SHIBOKEN_INCLUDE_DIR}
                                ${libpyside_SOURCE_DIR}
                                ${QtCore_BINARY_DIR}/PySide/QtCore/
                                ${QtGui_BINARY_DIR}/PySide/QtGui/
                                ${QtScript_BINARY_DIR}/PySide/QtScript/
                                ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtScriptTools)

set(QtScriptTools_libraries     pyside
                                ${SHIBOKEN_PYTHON_LIBRARIES}
                                ${Qt5Core_LIBRARIES}
                                ${Qt5Gui_LIBRARIES}
                                ${Qt5Script_LIBRARYIES}
                                ${Qt5ScriptTools_LIBRARYIES})
set(QtScriptTools_deps QtCore QtScript QtGui)

create_pyside_module(QtScriptTools
                     QtScriptTools_include_dirs
                     QtScriptTools_libraries
                     QtScriptTools_deps
                     QtScriptTools_typesystem_path
                     QtScriptTools_SRC
                     "")
