project(Qt3DAnimation)

set(Qt3DAnimation_SRC
${Qt3DAnimation_GEN_DIR}/qt3danimation_wrapper.cpp
${Qt3DAnimation_GEN_DIR}/qt3danimation_qabstractanimation_wrapper.cpp
${Qt3DAnimation_GEN_DIR}/qt3danimation_qabstractanimationclip_wrapper.cpp
${Qt3DAnimation_GEN_DIR}/qt3danimation_qabstractclipanimator_wrapper.cpp
${Qt3DAnimation_GEN_DIR}/qt3danimation_qabstractclipblendnode_wrapper.cpp
${Qt3DAnimation_GEN_DIR}/qt3danimation_qadditiveclipblend_wrapper.cpp
${Qt3DAnimation_GEN_DIR}/qt3danimation_qanimationaspect_wrapper.cpp
${Qt3DAnimation_GEN_DIR}/qt3danimation_qanimationcontroller_wrapper.cpp
${Qt3DAnimation_GEN_DIR}/qt3danimation_qanimationgroup_wrapper.cpp
${Qt3DAnimation_GEN_DIR}/qt3danimation_qblendedclipanimator_wrapper.cpp
${Qt3DAnimation_GEN_DIR}/qt3danimation_qclipanimator_wrapper.cpp
${Qt3DAnimation_GEN_DIR}/qt3danimation_qkeyframeanimation_wrapper.cpp
${Qt3DAnimation_GEN_DIR}/qt3danimation_qlerpclipblend_wrapper.cpp
${Qt3DAnimation_GEN_DIR}/qt3danimation_qmorphinganimation_wrapper.cpp
${Qt3DAnimation_GEN_DIR}/qt3danimation_qmorphtarget_wrapper.cpp
${Qt3DAnimation_GEN_DIR}/qt3danimation_qvertexblendanimation_wrapper.cpp
# module is always needed
${Qt3DAnimation_GEN_DIR}/qt3danimation_module_wrapper.cpp)

set(Qt3DAnimation_include_dirs
                     ${Qt3DAnimation_SOURCE_DIR}
                     ${Qt3DAnimation_BINARY_DIR}
                     ${pyside2_SOURCE_DIR}
                     ${Qt5Core_INCLUDE_DIRS}
                     ${Qt5Gui_INCLUDE_DIR}
                     ${Qt53DCore_INCLUDE_DIRS}
                     ${Qt53DRender_INCLUDE_DIRS}
                     ${Qt53DAnimation_INCLUDE_DIRS}
                     ${SHIBOKEN_INCLUDE_DIR}
                     ${libpyside_SOURCE_DIR}
                     ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                     ${QtCore_GEN_DIR}
                     ${QtGui_GEN_DIR}
                     ${Qt3DCore_GEN_DIR}
                     ${Qt3DRender_GEN_DIR}
                     ${Qt3DAnimation_GEN_DIR})

set(Qt3DAnimation_libraries pyside2
                     ${SHIBOKEN_PYTHON_LIBRARIES}
                     ${SHIBOKEN_LIBRARY}
                     ${Qt53DAnimation_LIBRARIES})

set(Qt3DAnimation_deps Qt3DRender)

create_pyside_module(Qt3DAnimation
                     Qt3DAnimation_include_dirs
                     Qt3DAnimation_libraries
                     Qt3DAnimation_deps
                     Qt3DAnimation_SOURCE_DIR
                     Qt3DAnimation_SRC
                     ""
                     ${Qt3DAnimation_BINARY_DIR}/typesystem_3danimation.xml)
