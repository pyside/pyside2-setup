project(QtSvg)

set(QtSvg_SRC
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSvg/qgraphicssvgitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSvg/qsvggenerator_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSvg/qsvgrenderer_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSvg/qsvgwidget_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSvg/qtsvg_module_wrapper.cpp
)

set(QtSvg_typesystem_path "${QtCore_SOURCE_DIR}${PATH_SEP}${QtCore_BINARY_DIR}${PATH_SEP}${QtGui_SOURCE_DIR}${PATH_SEP}${QtGui_BINARY_DIR}${QtWidgets_SOURCE_DIR}${PATH_SEP}${QtWidgets_BINARY_DIR}${PATH_SEP}${QtSvg_SOURCE_DIR}")
set(QtSvg_include_dirs  ${CMAKE_CURRENT_SOURCE_DIR}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${Qt5Gui_INCLUDE_DIRS}
                        ${Qt5Widgets_INCLUDE_DIRS}
                        ${Qt5Svg_INCLUDE_DIRS}
                        ${SHIBOKEN_INCLUDE_DIR}
                        ${libpyside_SOURCE_DIR}
                        ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                        ${QtCore_BINARY_DIR}/PySide/QtCore/
                        ${QtGui_BINARY_DIR}/PySide/QtGui/
                        ${QtWidgets_BINARY_DIR}/PySide/QtWidgets/
                        ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSvg/)
set(QtSvg_libraries     pyside
                        ${SHIBOKEN_PYTHON_LIBRARIES}
                        ${SHIBOKEN_LIBRARY}
                        ${Qt5Core_LIBRARIES}
                        ${Qt5Gui_LIBRARIES}
                        ${Qt5Widgets_LIBRARIES}
                        ${Qt5Svg_LIBRARIES}
                        )
set(QtSvg_deps "QtWidgets")
create_pyside_module(QtSvg
                     QtSvg_include_dirs
                     QtSvg_libraries
                     QtSvg_deps
                     QtSvg_typesystem_path
                     QtSvg_SRC
                     "")
