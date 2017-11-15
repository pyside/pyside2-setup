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
                     ${pyside2_SOURCE_DIR}
                     ${Qt5Core_INCLUDE_DIRS}
                     ${Qt5Gui_INCLUDE_DIRS}
                     ${SHIBOKEN_INCLUDE_DIR}
                     ${libpyside_SOURCE_DIR}
                     ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                     ${QtCore_GEN_DIR}
                     ${QtGui_GEN_DIR}
                     ${Qt3DCore_GEN_DIR})

set(Qt3DLogic_libraries pyside2
                     ${SHIBOKEN_PYTHON_LIBRARIES}
                     ${SHIBOKEN_LIBRARY}
                     ${Qt53DLogic_LIBRARIES})

set(Qt3DLogic_deps Qt3DCore)

create_pyside_module(Qt3DLogic
                     Qt3DLogic_include_dirs
                     Qt3DLogic_libraries
                     Qt3DLogic_deps
                     Qt3DLogic_SOURCE_DIR
                     Qt3DLogic_SRC
                     ""
                     ${Qt3DLogic_BINARY_DIR}/typesystem_3dlogic.xml)
