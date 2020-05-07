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

set(QtWebKitWidgets_include_dirs
                            ${QtWebKitWidgets_SOURCE_DIR}
                            ${QtWebKitWidgets_BINARY_DIR}
                            ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                            ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                            ${Qt${QT_MAJOR_VERSION}Widgets_INCLUDE_DIRS}
                            ${Qt${QT_MAJOR_VERSION}WebKit_INCLUDE_DIRS}
                            ${Qt${QT_MAJOR_VERSION}PrintSupport_INCLUDE_DIRS}
                            ${Qt${QT_MAJOR_VERSION}Network_INCLUDE_DIRS}
                            ${Qt${QT_MAJOR_VERSION}WebKitWidgets_INCLUDE_DIRS}
                            ${libpyside_SOURCE_DIR}
                            ${QtCore_GEN_DIR}
                            ${QtGui_GEN_DIR}
                            ${QtWidgets_GEN_DIR}
                            ${QtPrintSupport_GEN_DIR}
                            ${QtWebKit_GEN_DIR}
                            ${QtNetwork_GEN_DIR}
                            )
set(QtWebKitWidgets_libraries      pyside2
                            ${Qt${QT_MAJOR_VERSION}WebKit_LIBRARIES}
                            ${Qt${QT_MAJOR_VERSION}WebKitWidgets_LIBRARIES}
                            ${Qt${QT_MAJOR_VERSION}Network_LIBRARIES}
                            ${Qt${QT_MAJOR_VERSION}Widgets_LIBRARIES}
                            ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES}
                            ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                            )
set(QtWebKitWidgets_deps QtWidgets QtPrintSupport QtNetwork)
create_pyside_module(NAME QtWebKitWidgets
                     INCLUDE_DIRS QtWebKitWidgets_include_dirs
                     LIBRARIES QtWebKitWidgets_libraries
                     DEPS QtWebKitWidgets_deps
                     TYPESYSTEM_PATH QtWebKitWidgets_SOURCE_DIR
                     SOURCES QtWebKitWidgets_SRC)

