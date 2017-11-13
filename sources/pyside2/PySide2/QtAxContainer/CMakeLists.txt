project(QtAxContainer)

set(QtAxContainer_SRC
${QtAxContainer_GEN_DIR}/qaxbase_wrapper.cpp
${QtAxContainer_GEN_DIR}/qaxobject_wrapper.cpp
${QtAxContainer_GEN_DIR}/qaxscript_wrapper.cpp
${QtAxContainer_GEN_DIR}/qaxscriptengine_wrapper.cpp
${QtAxContainer_GEN_DIR}/qaxscriptmanager_wrapper.cpp
${QtAxContainer_GEN_DIR}/qaxselect_wrapper.cpp
${QtAxContainer_GEN_DIR}/qaxwidget_wrapper.cpp
# module is always needed
${QtAxContainer_GEN_DIR}/qtaxcontainer_module_wrapper.cpp)

configure_file("${QtAxContainer_SOURCE_DIR}/QtAxContainer_global.post.h.in"
               "${QtAxContainer_BINARY_DIR}/QtAxContainer_global.post.h" @ONLY)

set(QtAxContainer_include_dirs
                     ${QtAxContainer_SOURCE_DIR}
                     ${QtAxContainer_BINARY_DIR}
                     ${pyside2_SOURCE_DIR}
                     ${Qt5Core_INCLUDE_DIRS}
                     ${Qt5Gui_INCLUDE_DIRS}
                     ${Qt5Widgets_INCLUDE_DIRS}
                     ${SHIBOKEN_INCLUDE_DIR}
                     ${libpyside_SOURCE_DIR}
                     ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                     ${QtCore_GEN_DIR}
                     ${QtGui_GEN_DIR}
                     ${QtWidgets_GEN_DIR})

set(QtAxContainer_libraries pyside2
                     ${SHIBOKEN_PYTHON_LIBRARIES}
                     ${SHIBOKEN_LIBRARY}
                     ${Qt5AxContainer_LIBRARIES}
                     ${Qt5AxBase_LIBRARIES})

set(QtAxContainer_deps QtWidgets)

create_pyside_module(QtAxContainer
                     QtAxContainer_include_dirs
                     QtAxContainer_libraries
                     QtAxContainer_deps
                     QtAxContainer_SOURCE_DIR
                     QtAxContainer_SRC
                     ""
                     ${QtAxContainer_BINARY_DIR}/typesystem_axcontainer.xml)
