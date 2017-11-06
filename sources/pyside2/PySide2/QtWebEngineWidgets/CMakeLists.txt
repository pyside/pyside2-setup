project(QtWebEngineWidgets)

set(QtWebEngineWidgets_SRC
${QtWebEngineWidgets_GEN_DIR}/qwebenginecertificateerror_wrapper.cpp
${QtWebEngineWidgets_GEN_DIR}/qwebenginedownloaditem_wrapper.cpp
#${QtWebEngineWidgets_GEN_DIR}/qwebenginehistory_wrapper.cpp
${QtWebEngineWidgets_GEN_DIR}/qwebenginehistoryitem_wrapper.cpp
${QtWebEngineWidgets_GEN_DIR}/qwebenginepage_wrapper.cpp
${QtWebEngineWidgets_GEN_DIR}/qwebengineprofile_wrapper.cpp
${QtWebEngineWidgets_GEN_DIR}/qwebenginescript_wrapper.cpp
#${QtWebEngineWidgets_GEN_DIR}/qwebenginescriptcollection_wrapper.cpp
#${QtWebEngineWidgets_GEN_DIR}/qwebenginesettings_wrapper.cpp
${QtWebEngineWidgets_GEN_DIR}/qwebengineview_wrapper.cpp
# module is always needed
${QtWebEngineWidgets_GEN_DIR}/qtwebenginewidgets_module_wrapper.cpp
)

set(QtWebEngineWidgets_include_dirs
                            ${QtWebEngineWidgets_SOURCE_DIR}
                            ${QtWebEngineWidgets_BINARY_DIR}
                            ${Qt5Core_INCLUDE_DIRS}
                            ${Qt5Gui_INCLUDE_DIRS}
                            ${Qt5Widgets_INCLUDE_DIRS}
                            ${Qt5Network_INCLUDE_DIRS}
                            ${Qt5WebChannel_INCLUDE_DIRS}
                            ${Qt5WebEngineWidgets_INCLUDE_DIRS}
                            ${SHIBOKEN_INCLUDE_DIR}
                            ${libpyside_SOURCE_DIR}
                            ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                            ${QtCore_GEN_DIR}
                            ${QtGui_GEN_DIR}
                            ${QtWidgets_GEN_DIR}
                            ${QtWebEngineWidgets_GEN_DIR}
                            ${QtNetwork_GEN_DIR}
                            ${QtWebChannel_GEN_DIR}
                            )
set(QtWebEngineWidgets_libraries      pyside2
                            ${SHIBOKEN_PYTHON_LIBRARIES}
                            ${SHIBOKEN_LIBRARY}
                            ${Qt5WebEngineWidgets_LIBRARIES}
                            ${Qt5Network_LIBRARIES}
                            ${Qt5WebChannel_LIBRARIES}
                            ${Qt5Widgets_LIBRARIES}
                            ${Qt5Gui_LIBRARIES}
                            ${Qt5Core_LIBRARIES}
                            )
set(QtWebEngineWidgets_deps QtGui QtWidgets QtNetwork QtWebChannel)
create_pyside_module(QtWebEngineWidgets
                     QtWebEngineWidgets_include_dirs
                     QtWebEngineWidgets_libraries
                     QtWebEngineWidgets_deps
                     QtWebEngineWidgets_SOURCE_DIR
                     QtWebEngineWidgets_SRC
                     "")

