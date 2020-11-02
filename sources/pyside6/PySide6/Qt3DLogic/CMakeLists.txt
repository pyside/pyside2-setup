project(Qt3DLogic)

set(Qt3DLogic_SRC
${Qt3DLogic_GEN_DIR}/qt3dlogic_wrapper.cpp
${Qt3DLogic_GEN_DIR}/qt3dlogic_qframeaction_wrapper.cpp
${Qt3DLogic_GEN_DIR}/qt3dlogic_qlogicaspect_wrapper.cpp
# module is always needed
${Qt3DLogic_GEN_DIR}/qt3dlogic_module_wrapper.cpp)

set(Qt3DLogic_include_dirs
                     ${Qt3DLogic_SOURCE_DIR}
                     ${Qt3DLogic_BINARY_DIR}
                     ${pyside6_SOURCE_DIR}
                     ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                     ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                     ${libpyside_SOURCE_DIR}
                     ${QtCore_GEN_DIR}
                     ${QtGui_GEN_DIR}
                     ${Qt3DCore_GEN_DIR})

set(Qt3DLogic_libraries pyside6
                     ${Qt${QT_MAJOR_VERSION}3DLogic_LIBRARIES})

set(Qt3DLogic_deps Qt3DCore)

create_pyside_module(NAME Qt3DLogic
                     INCLUDE_DIRS Qt3DLogic_include_dirs
                     LIBRARIES Qt3DLogic_libraries
                     DEPS Qt3DLogic_deps
                     TYPESYSTEM_PATH Qt3DLogic_SOURCE_DIR
                     SOURCES Qt3DLogic_SRC
                     TYPESYSTEM_NAME ${Qt3DLogic_BINARY_DIR}/typesystem_3dlogic.xml)
