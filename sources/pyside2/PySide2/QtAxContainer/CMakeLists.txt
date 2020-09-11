project(QtAxContainer)

set(QtAxContainer_SRC
${QtAxContainer_GEN_DIR}/qaxbase_wrapper.cpp
${QtAxContainer_GEN_DIR}/qaxbaseobject_wrapper.cpp
${QtAxContainer_GEN_DIR}/qaxbasewidget_wrapper.cpp
${QtAxContainer_GEN_DIR}/qaxobject_wrapper.cpp
${QtAxContainer_GEN_DIR}/qaxobjectinterface_wrapper.cpp
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
                     ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                     ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                     ${Qt${QT_MAJOR_VERSION}Widgets_INCLUDE_DIRS}
                     ${libpyside_SOURCE_DIR}
                     ${QtCore_GEN_DIR}
                     ${QtGui_GEN_DIR}
                     ${QtWidgets_GEN_DIR})

set(QtAxContainer_libraries pyside2
                     ${Qt${QT_MAJOR_VERSION}AxContainer_LIBRARIES}
                     ${Qt${QT_MAJOR_VERSION}AxBase_LIBRARIES})

set(QtAxContainer_deps QtWidgets)

create_pyside_module(NAME QtAxContainer
                     INCLUDE_DIRS QtAxContainer_include_dirs
                     LIBRARIES QtAxContainer_libraries
                     DEPS QtAxContainer_deps
                     TYPESYSTEM_PATH QtAxContainer_SOURCE_DIR
                     SOURCES QtAxContainer_SRC
                     TYPESYSTEM_NAME ${QtAxContainer_BINARY_DIR}/typesystem_axcontainer.xml)
