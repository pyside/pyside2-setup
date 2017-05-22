project(QtSvg)

set(QtSvg_SRC
${QtSvg_GEN_DIR}/qgraphicssvgitem_wrapper.cpp
${QtSvg_GEN_DIR}/qsvggenerator_wrapper.cpp
${QtSvg_GEN_DIR}/qsvgrenderer_wrapper.cpp
${QtSvg_GEN_DIR}/qsvgwidget_wrapper.cpp
# module is always needed
${QtSvg_GEN_DIR}/qtsvg_module_wrapper.cpp
)

make_path(QtSvg_typesystem_path ${QtCore_SOURCE_DIR} ${QtGui_SOURCE_DIR} ${QtWidgets_SOURCE_DIR}
                                ${QtCore_BINARY_DIR} ${QtGui_BINARY_DIR} ${QtWidgets_BINARY_DIR}
                                ${QtSvg_SOURCE_DIR})

set(QtSvg_include_dirs  ${QtSvg_SOURCE_DIR}
                        ${QtSvg_BINARY_DIR}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${Qt5Gui_INCLUDE_DIRS}
                        ${Qt5Widgets_INCLUDE_DIRS}
                        ${Qt5Svg_INCLUDE_DIRS}
                        ${SHIBOKEN_INCLUDE_DIR}
                        ${libpyside_SOURCE_DIR}
                        ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                        ${QtCore_GEN_DIR}
                        ${QtGui_GEN_DIR}
                        ${QtWidgets_GEN_DIR}
                        )
set(QtSvg_libraries     pyside2
                        ${SHIBOKEN_PYTHON_LIBRARIES}
                        ${SHIBOKEN_LIBRARY}
                        ${Qt5Core_LIBRARIES}
                        ${Qt5Gui_LIBRARIES}
                        ${Qt5Widgets_LIBRARIES}
                        ${Qt5Svg_LIBRARIES}
                        )
set(QtSvg_deps QtWidgets)

create_pyside_module(QtSvg
                     QtSvg_include_dirs
                     QtSvg_libraries
                     QtSvg_deps
                     QtSvg_typesystem_path
                     QtSvg_SRC
                     "")
