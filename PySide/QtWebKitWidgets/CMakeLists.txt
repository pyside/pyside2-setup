project(QtWebKitWidgets)

set(QtWebKitWidgets_SRC
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKitWidgets/qgraphicswebview_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKitWidgets/qwebframe_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKitWidgets/qwebhittestresult_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKitWidgets/qwebinspector_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKitWidgets/qwebpage_choosemultiplefilesextensionoption_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKitWidgets/qwebpage_choosemultiplefilesextensionreturn_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKitWidgets/qwebpage_errorpageextensionoption_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKitWidgets/qwebpage_errorpageextensionreturn_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKitWidgets/qwebpage_extensionoption_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKitWidgets/qwebpage_extensionreturn_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKitWidgets/qwebpage_viewportattributes_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKitWidgets/qwebpage_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKitWidgets/qwebview_wrapper.cpp
# this file is always needed
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKitWidgets/qtwebkitwidgets_module_wrapper.cpp
)

# fake these variables for qt5
set(QtWebKitWidgets_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(QtWebKitWidgets_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKitWidgets)

make_path(QtWebKitWidgets_typesystem_path 
                            ${QtCore_SOURCE_DIR} ${QtGui_SOURCE_DIR} ${QtWidgets_SOURCE_DIR}
                            ${QtCore_BINARY_DIR} ${QtGui_BINARY_DIR} ${QtWidgets_BINARY_DIR}
                            ${QtWebKit_SOURCE_DIR} ${QtNetwork_SOURCE_DIR}
                            ${QtWebKit_BINARY_DIR} ${QtNetwork_BINARY_DIR}
                            ${QtWebKitWidgets_SOURCE_DIR})

set(QtWebKitWidgets_include_dirs
                            ${CMAKE_CURRENT_SOURCE_DIR}
                            ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtWebKit/
                            ${Qt5Core_INCLUDE_DIRS}
                            ${Qt5Gui_INCLUDE_DIRS}
                            ${Qt5Widgets_INCLUDE_DIRS}
                            ${Qt5WebKit_INCLUDE_DIRS}
                            ${Qt5Network_INCLUDE_DIRS}
                            ${Qt5WebKitWidgets_INCLUDE_DIRS}
                            ${SHIBOKEN_INCLUDE_DIR}
                            ${libpyside_SOURCE_DIR}
                            ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                            ${QtCore_BINARY_DIR}/PySide/QtCore/
                            ${QtGui_BINARY_DIR}/PySide/QtGui/
                            ${QtWidgets_BINARY_DIR}/PySide/QtWidgets/
                            ${QtWebKit_BINARY_DIR}/PySide/QtWebKit/
                            ${QtNetwork_BINARY_DIR}/PySide/QtNetwork/ )
set(QtWebKitWidgets_libraries      pyside
                            ${SHIBOKEN_PYTHON_LIBRARIES}
                            ${SHIBOKEN_LIBRARY}
                            ${Qt5WebkitWidgets_LIBRARIES}
                            ${Qt5Network_LIBRARIES}
                            ${Qt5Widgets_LIBRARIES}
                            ${Qt5Gui_LIBRARIES}
                            ${Qt5Core_LIBRARIES}
                            )
set(QtWebKitWidgets_deps QtWidgets QtNetwork)
create_pyside_module(QtWebKitWidgets
                     QtWebKitWidgets_include_dirs
                     QtWebKitWidgets_libraries
                     QtWebKitWidgets_deps
                     QtWebKitWidgets_typesystem_path
                     QtWebKitWidgets_SRC
                     "")

