project(QtSvgWidgets)

set(QtSvgWidgets_SRC
${QtSvgWidgets_GEN_DIR}/qgraphicssvgitem_wrapper.cpp
${QtSvgWidgets_GEN_DIR}/qsvgwidget_wrapper.cpp
# module is always needed
${QtSvgWidgets_GEN_DIR}/qtsvgwidgets_module_wrapper.cpp
)

set(QtSvgWidgets_include_dirs ${QtSvgWidgets_SOURCE_DIR}
                              ${QtSvgWidgets_BINARY_DIR}
                              ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                              ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                              ${Qt${QT_MAJOR_VERSION}Widgets_INCLUDE_DIRS}
                              ${Qt${QT_MAJOR_VERSION}Svg_INCLUDE_DIRS}
                              ${Qt${QT_MAJOR_VERSION}SvgWidgets_INCLUDE_DIRS}
                              ${libpyside_SOURCE_DIR}
                              ${QtCore_GEN_DIR}
                              ${QtGui_GEN_DIR}
                              ${QtWidgets_GEN_DIR}
                              ${QtSvg_GEN_DIR}
                              )

set(QtSvgWidgets_libraries pyside2
                           ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                           ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES}
                           ${Qt${QT_MAJOR_VERSION}Widgets_LIBRARIES}
                           ${Qt${QT_MAJOR_VERSION}Svg_LIBRARIES}
                           ${Qt${QT_MAJOR_VERSION}SvgWidgets_LIBRARIES}
                           )

set(QtSvgWidgets_deps QtSvg QtWidgets)

create_pyside_module(NAME QtSvgWidgets
                     INCLUDE_DIRS QtSvgWidgets_include_dirs
                     LIBRARIES QtSvgWidgets_libraries
                     DEPS QtSvgWidgets_deps
                     TYPESYSTEM_PATH QtSvgWidgets_SOURCE_DIR
                     SOURCES QtSvgWidgets_SRC)
