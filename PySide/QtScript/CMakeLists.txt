project(QtScript)

set(QtScript_SRC
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtScript/qscriptable_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtScript/qscriptclass_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtScript/qscriptclasspropertyiterator_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtScript/qscriptcontext_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtScript/qscriptcontextinfo_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtScript/qscriptengineagent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtScript/qscriptprogram_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtScript/qscriptengine_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtScript/qscriptextensioninterface_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtScript/qscriptextensionplugin_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtScript/qscriptstring_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtScript/qscriptvalue_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtScript/qscriptvalueiterator_wrapper.cpp
# module is always needed
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtScript/qtscript_module_wrapper.cpp
)

make_path(QtScript_typesystem_path ${QtCore_SOURCE_DIR} ${QtCore_BINARY_DIR})

set(QtScript_include_dirs   ${QtScript_SOURCE_DIR}
                            ${QtScript_BINARY_DIR}
                            ${Qt5Core_INCLUDE_DIRS}
                            ${Qt5Script_INCLUDE_DIRS}
                            ${SHIBOKEN_INCLUDE_DIR}
                            ${libpyside_SOURCE_DIR}
                            ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                            ${QtCore_BINARY_DIR}/PySide/QtCore )
set(QtScript_libraries      pyside
                            ${SHIBOKEN_PYTHON_LIBRARIES}
                            ${SHIBOKEN_LIBRARY}
                            ${Qt5Core_LIBRARIES}
                            ${Qt5Script_LIBRARIES})
set(QtScript_deps QtCore)
create_pyside_module(QtScript
                     QtScript_include_dirs
                     QtScript_libraries
                     QtScript_deps
                     QtScript_typesystem_path
                     QtScript_SRC
                     "")
