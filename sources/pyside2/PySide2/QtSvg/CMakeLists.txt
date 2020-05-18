project(QtSvg)

set(QtSvg_SRC
${QtSvg_GEN_DIR}/qgraphicssvgitem_wrapper.cpp
${QtSvg_GEN_DIR}/qsvggenerator_wrapper.cpp
${QtSvg_GEN_DIR}/qsvgrenderer_wrapper.cpp
${QtSvg_GEN_DIR}/qsvgwidget_wrapper.cpp
# module is always needed
${QtSvg_GEN_DIR}/qtsvg_module_wrapper.cpp
)

set(QtSvg_include_dirs  ${QtSvg_SOURCE_DIR}
                        ${QtSvg_BINARY_DIR}
                        ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                        ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                        ${Qt${QT_MAJOR_VERSION}Widgets_INCLUDE_DIRS}
                        ${Qt${QT_MAJOR_VERSION}Svg_INCLUDE_DIRS}
                        ${libpyside_SOURCE_DIR}
                        ${QtCore_GEN_DIR}
                        ${QtGui_GEN_DIR}
                        ${QtWidgets_GEN_DIR}
                        )
set(QtSvg_libraries     pyside2
                        ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                        ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES}
                        ${Qt${QT_MAJOR_VERSION}Widgets_LIBRARIES}
                        ${Qt${QT_MAJOR_VERSION}Svg_LIBRARIES}
                        )
set(QtSvg_deps QtWidgets)

create_pyside_module(NAME QtSvg
                     INCLUDE_DIRS QtSvg_include_dirs
                     LIBRARIES QtSvg_libraries
                     DEPS QtSvg_deps
                     TYPESYSTEM_PATH QtSvg_SOURCE_DIR
                     SOURCES QtSvg_SRC)
