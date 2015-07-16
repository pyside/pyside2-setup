project(QtHelp)

set(QtHelp_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtHelp/qhelpcontentitem_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtHelp/qhelpcontentmodel_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtHelp/qhelpcontentwidget_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtHelp/qhelpenginecore_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtHelp/qhelpengine_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtHelp/qhelpindexmodel_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtHelp/qhelpindexwidget_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtHelp/qhelpsearchengine_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtHelp/qhelpsearchquerywidget_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtHelp/qhelpsearchquery_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtHelp/qhelpsearchresultwidget_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtHelp/qthelp_module_wrapper.cpp
)

make_path(QtHelp_typesystem_path ${QtCore_SOURCE_DIR} ${QtGui_SOURCE_DIR} ${QtWidgets_SOURCE_DIR}
                                 ${QtCore_BINARY_DIR} ${QtGui_BINARY_DIR} ${QtWidgets_BINARY_DIR}
                                 ${QtHelp_SOURCE_DIR}${PATH_SEP})
set(QtHelp_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${Qt5Gui_INCLUDE_DIRS}
                        ${Qt5Help_INCLUDE_DIRS}
                        ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                        ${SHIBOKEN_INCLUDE_DIR}
                        ${libpyside_SOURCE_DIR}
                        ${QtGui_BINARY_DIR}/PySide/QtGui/
                        ${QtCore_BINARY_DIR}/PySide/QtCore/
                        ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtHelp)
set(QtHelp_libraries  pyside
                      ${SHIBOKEN_PYTHON_LIBRARIES}
                      ${Qt5Core_LIBRARIes}
                      ${Qt5Gui_LIBRARIES}
                      ${Qt5Help_LIBRARIES})
set(QtHelp_deps QtWidgets)

create_pyside_module(QtHelp
                     QtHelp_include_dirs
                     QtHelp_libraries
                     QtHelp_deps
                     QtHelp_typesystem_path
                     QtHelp_SRC
                     "")
