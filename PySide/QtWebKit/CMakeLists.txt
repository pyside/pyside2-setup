project(QtWebKit)

set(QtWebKit_SRC
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebdatabase_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebelement_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebelementcollection_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebhistory_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebhistoryinterface_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebhistoryitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebpluginfactory_mimetype_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebpluginfactory_plugin_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebpluginfactory_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebsecurityorigin_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qwebsettings_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/webcore_wrapper.cpp

# this file is always needed
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/qtwebkit_module_wrapper.cpp
)

# fake these variables for qt5
set(QtWebKit_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(QtWebKit_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit)

make_path(QtWebKit_typesystem_path ${QtCore_SOURCE_DIR} ${QtGui_SOURCE_DIR} ${QtNetwork_SOURCE_DIR}
                                   ${QtCore_BINARY_DIR} ${QtGui_BINARY_DIR} ${QtNetwork_BINARY_DIR}
                                   ${QtWebKit_SOURCE_DIR})

set(QtWebKit_include_dirs   ${CMAKE_CURRENT_SOURCE_DIR}
                            ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/
                            ${Qt5Core_INCLUDE_DIRS}
                            ${Qt5Gui_INCLUDE_DIRS}
                            ${Qt5Network_INCLUDE_DIRS}
                            ${Qt5WebKit_INCLUDE_DIRS}
                            ${SHIBOKEN_INCLUDE_DIR}
                            ${libpyside_SOURCE_DIR}
                            ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                            ${QtCore_BINARY_DIR}/PySide/QtCore/
                            ${QtGui_BINARY_DIR}/PySide/QtGui/
                            ${QtWidgets_BINARY_DIR}/PySide/QtWidgets/
                            ${QtNetwork_BINARY_DIR}/PySide/QtNetwork/ )
set(QtWebKit_libraries      pyside
                            ${SHIBOKEN_PYTHON_LIBRARIES}
                            ${SHIBOKEN_LIBRARY}
                            ${Qt5Webkit_LIBRARIES}
                            ${Qt5Network_LIBRARIES}
                            ${Qt5Gui_LIBRARIES}
                            ${Qt5Core_LIBRARIES}
                            )
set(QtWebKit_deps QtWidgets QtNetwork)
create_pyside_module(QtWebKit
                     QtWebKit_include_dirs
                     QtWebKit_libraries
                     QtWebKit_deps
                     QtWebKit_typesystem_path
                     QtWebKit_SRC
                     "")

