project(QtWebKitWidgets)

set(QtWebKitWidgets_SRC
${QtWebKitWidgets_GEN_DIR}/qgraphicswebview_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebdatabase_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebelement_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebelementcollection_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebframe_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebhistory_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebhistoryinterface_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebhistoryitem_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebhittestresult_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebinspector_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebpage_choosemultiplefilesextensionoption_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebpage_choosemultiplefilesextensionreturn_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebpage_errorpageextensionoption_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebpage_errorpageextensionreturn_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebpage_extensionoption_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebpage_extensionreturn_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebpage_viewportattributes_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebpage_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebpluginfactory_mimetype_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebpluginfactory_plugin_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebpluginfactory_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebsecurityorigin_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebsettings_wrapper.cpp
${QtWebKitWidgets_GEN_DIR}/qwebview_wrapper.cpp
# module is always needed
${QtWebKitWidgets_GEN_DIR}/qtwebkitwidgets_module_wrapper.cpp
)

make_path(QtWebKitWidgets_typesystem_path 
                            ${QtCore_SOURCE_DIR} ${QtGui_SOURCE_DIR} ${QtWidgets_SOURCE_DIR}
                            ${QtCore_BINARY_DIR} ${QtGui_BINARY_DIR} ${QtWidgets_BINARY_DIR}
                            ${QtWebKit_SOURCE_DIR} ${QtPrintSupport_SOURCE_DIR} ${QtNetwork_SOURCE_DIR}
                            ${QtWebKit_BINARY_DIR} ${QtPrintSupport_BINARY_DIR} ${QtNetwork_BINARY_DIR}
                            ${QtWebKitWidgets_SOURCE_DIR})

set(QtWebKitWidgets_include_dirs
                            ${QtWebKitWidgets_SOURCE_DIR}
                            ${QtWebKitWidgets_BINARY_DIR}
                            ${Qt5Core_INCLUDE_DIRS}
                            ${Qt5Gui_INCLUDE_DIRS}
                            ${Qt5Widgets_INCLUDE_DIRS}
                            ${Qt5WebKit_INCLUDE_DIRS}
                            ${Qt5PrintSupport_INCLUDE_DIRS}
                            ${Qt5Network_INCLUDE_DIRS}
                            ${Qt5WebKitWidgets_INCLUDE_DIRS}
                            ${SHIBOKEN_INCLUDE_DIR}
                            ${libpyside_SOURCE_DIR}
                            ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                            ${QtCore_GEN_DIR}
                            ${QtGui_GEN_DIR}
                            ${QtWidgets_GEN_DIR}
                            ${QtPrintSupport_GEN_DIR}
                            ${QtWebKit_GEN_DIR}
                            ${QtNetwork_GEN_DIR}
                            )
set(QtWebKitWidgets_libraries      pyside2
                            ${SHIBOKEN_PYTHON_LIBRARIES}
                            ${SHIBOKEN_LIBRARY}
                            ${Qt5WebKit_LIBRARIES}
                            ${Qt5WebKitWidgets_LIBRARIES}
                            ${Qt5Network_LIBRARIES}
                            ${Qt5Widgets_LIBRARIES}
                            ${Qt5Gui_LIBRARIES}
                            ${Qt5Core_LIBRARIES}
                            )
set(QtWebKitWidgets_deps QtWidgets QtPrintSupport QtNetwork)
create_pyside_module(QtWebKitWidgets
                     QtWebKitWidgets_include_dirs
                     QtWebKitWidgets_libraries
                     QtWebKitWidgets_deps
                     QtWebKitWidgets_typesystem_path
                     QtWebKitWidgets_SRC
                     "")

