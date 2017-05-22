project(QtMultimediaWidgets)

set(QtMultimediaWidgets_SRC
${QtMultimediaWidgets_GEN_DIR}/qcameraviewfinder_wrapper.cpp
${QtMultimediaWidgets_GEN_DIR}/qgraphicsvideoitem_wrapper.cpp
${QtMultimediaWidgets_GEN_DIR}/qvideowidget_wrapper.cpp
${QtMultimediaWidgets_GEN_DIR}/qvideowidgetcontrol_wrapper.cpp
# module is always needed
${QtMultimediaWidgets_GEN_DIR}/qtmultimediawidgets_module_wrapper.cpp
)

make_path(QtMultimediaWidgets_typesystem_path
    ${QtCore_SOURCE_DIR} ${QtGui_SOURCE_DIR} ${QtNetwork_SOURCE_DIR} ${QtWidgets_SOURCE_DIR} ${QtMultimedia_SOURCE_DIR}
    ${QtCore_BINARY_DIR} ${QtGui_BINARY_DIR} ${QtNetwork_BINARY_DIR} ${QtWidgets_BINARY_DIR} ${QtMultimedia_BINARY_DIR}
    ${QtMultimediaWidgets_SOURCE_DIR})

set(QtMultimediaWidgets_include_dirs ${QtMultimediaWidgets_SOURCE_DIR}
                                     ${QtMultimediaWidgets_BINARY_DIR}
                                     ${Qt5Core_INCLUDE_DIRS}
                                     ${Qt5Gui_INCLUDE_DIRS}
                                     ${Qt5Network_INCLUDE_DIRS}
                                     ${Qt5Widgets_INCLUDE_DIRS}
                                     ${Qt5Multimedia_INCLUDE_DIRS}
                                     ${Qt5MultimediaWidgets_INCLUDE_DIRS}
                                     ${SHIBOKEN_INCLUDE_DIR}
                                     ${libpyside_SOURCE_DIR}
                                     ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                                     ${QtCore_GEN_DIR}
                                     ${QtGui_GEN_DIR}
                                     ${QtNetwork_GEN_DIR}
                                     ${QtWidgets_GEN_DIR}
                                     ${QtMultimedia_GEN_DIR})

set(QtMultimediaWidgets_libraries    pyside2
                                     ${SHIBOKEN_PYTHON_LIBRARIES}
                                     ${SHIBOKEN_LIBRARY}
                                     ${Qt5Multimedia_LIBRARIES}
                                     ${Qt5MultimediaWidgets_LIBRARIES}
                                     ${Qt5Core_LIBRARIES}
                                     ${Qt5Gui_LIBRARIES}
                                     ${Qt5Network_LIBRARIES}
                                     ${Qt5Widgets_LIBRARIES})

set(QtMultimediaWidgets_deps QtCore QtGui QtNetwork QtWidgets QtMultimedia)

create_pyside_module(QtMultimediaWidgets
                     QtMultimediaWidgets_include_dirs
                     QtMultimediaWidgets_libraries
                     QtMultimediaWidgets_deps
                     QtMultimediaWidgets_typesystem_path
                     QtMultimediaWidgets_SRC
                     "")
