project(QtMultimediaWidgets)

set(QtMultimediaWidgets_SRC
${QtMultimediaWidgets_GEN_DIR}/qcameraviewfinder_wrapper.cpp
${QtMultimediaWidgets_GEN_DIR}/qgraphicsvideoitem_wrapper.cpp
${QtMultimediaWidgets_GEN_DIR}/qvideowidget_wrapper.cpp
${QtMultimediaWidgets_GEN_DIR}/qvideowidgetcontrol_wrapper.cpp
# module is always needed
${QtMultimediaWidgets_GEN_DIR}/qtmultimediawidgets_module_wrapper.cpp
)

set(QtMultimediaWidgets_include_dirs ${QtMultimediaWidgets_SOURCE_DIR}
                                     ${QtMultimediaWidgets_BINARY_DIR}
                                     ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                                     ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                                     ${Qt${QT_MAJOR_VERSION}Network_INCLUDE_DIRS}
                                     ${Qt${QT_MAJOR_VERSION}Widgets_INCLUDE_DIRS}
                                     ${Qt${QT_MAJOR_VERSION}Multimedia_INCLUDE_DIRS}
                                     ${Qt${QT_MAJOR_VERSION}MultimediaWidgets_INCLUDE_DIRS}
                                     ${libpyside_SOURCE_DIR}
                                     ${QtCore_GEN_DIR}
                                     ${QtGui_GEN_DIR}
                                     ${QtNetwork_GEN_DIR}
                                     ${QtWidgets_GEN_DIR}
                                     ${QtMultimedia_GEN_DIR})

set(QtMultimediaWidgets_libraries    pyside6
                                     ${Qt${QT_MAJOR_VERSION}Multimedia_LIBRARIES}
                                     ${Qt${QT_MAJOR_VERSION}MultimediaWidgets_LIBRARIES}
                                     ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                                     ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES}
                                     ${Qt${QT_MAJOR_VERSION}Network_LIBRARIES}
                                     ${Qt${QT_MAJOR_VERSION}Widgets_LIBRARIES})

set(QtMultimediaWidgets_deps QtCore QtGui QtNetwork QtWidgets QtMultimedia)

create_pyside_module(NAME QtMultimediaWidgets
                     INCLUDE_DIRS QtMultimediaWidgets_include_dirs
                     LIBRARIES QtMultimediaWidgets_libraries
                     DEPS QtMultimediaWidgets_deps
                     TYPESYSTEM_PATH QtMultimediaWidgets_SOURCE_DIR
                     SOURCES QtMultimediaWidgets_SRC)
