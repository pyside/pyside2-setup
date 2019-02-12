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
                     ${libpyside_SOURCE_DIR}
                     ${QtCore_GEN_DIR}
                     ${QtGui_GEN_DIR}
                     ${Qt3DCore_GEN_DIR}
                     ${Qt3DRender_GEN_DIR}
                     ${Qt3DAnimation_GEN_DIR})

set(Qt3DAnimation_libraries pyside2
                     ${Qt53DAnimation_LIBRARIES})

set(Qt3DAnimation_deps Qt3DRender)

create_pyside_module(NAME Qt3DAnimation
                     INCLUDE_DIRS Qt3DAnimation_include_dirs
                     LIBRARIES Qt3DAnimation_libraries
                     DEPS Qt3DAnimation_deps
                     TYPESYSTEM_PATH Qt3DAnimation_SOURCE_DIR
                     SOURCES Qt3DAnimation_SRC
                     TYPESYSTEM_NAME ${Qt3DAnimation_BINARY_DIR}/typesystem_3danimation.xml)
