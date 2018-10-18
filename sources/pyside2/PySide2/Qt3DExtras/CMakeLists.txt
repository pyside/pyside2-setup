project(Qt3DExtras)

set(Qt3DExtras_SRC
${Qt3DExtras_GEN_DIR}/qt3dextras_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qconegeometry_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qconemesh_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qcuboidgeometry_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qcuboidmesh_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qcylindergeometry_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qcylindermesh_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qdiffusemapmaterial_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qdiffusespecularmapmaterial_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qextrudedtextgeometry_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qextrudedtextmesh_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qfirstpersoncameracontroller_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qforwardrenderer_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qgoochmaterial_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qmetalroughmaterial_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qmorphphongmaterial_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qnormaldiffusemapmaterial_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qnormaldiffusespecularmapmaterial_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qorbitcameracontroller_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qpervertexcolormaterial_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qphongmaterial_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qphongalphamaterial_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qplanegeometry_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qplanemesh_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qskyboxentity_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qspheregeometry_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qspheremesh_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qtext2dentity_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qtexturematerial_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qtorusgeometry_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qtorusmesh_wrapper.cpp
${Qt3DExtras_GEN_DIR}/qt3dextras_qt3dwindow_wrapper.cpp
# module is always needed
${Qt3DExtras_GEN_DIR}/qt3dextras_module_wrapper.cpp)

if (Qt53DExtras_VERSION VERSION_EQUAL 5.10.0 OR Qt53DExtras_VERSION VERSION_GREATER 5.10.0)
    list(APPEND Qt3DExtras_SRC
         ${Qt3DExtras_GEN_DIR}/qt3dextras_qabstractcameracontroller_wrapper.cpp
         ${Qt3DExtras_GEN_DIR}/qt3dextras_qabstractcameracontroller_inputstate_wrapper.cpp
         ${Qt3DExtras_GEN_DIR}/qt3dextras_qabstractspritesheet_wrapper.cpp
         ${Qt3DExtras_GEN_DIR}/qt3dextras_qdiffusespecularmaterial_wrapper.cpp
         ${Qt3DExtras_GEN_DIR}/qt3dextras_qspritegrid_wrapper.cpp
         ${Qt3DExtras_GEN_DIR}/qt3dextras_qspritesheet_wrapper.cpp
         ${Qt3DExtras_GEN_DIR}/qt3dextras_qspritesheetitem_wrapper.cpp)
endif()

set(Qt3DExtras_include_dirs
                     ${Qt3DExtras_SOURCE_DIR}
                     ${Qt3DExtras_BINARY_DIR}
                     ${pyside2_SOURCE_DIR}
                     ${Qt5Core_INCLUDE_DIRS}
                     ${Qt5Gui_INCLUDE_DIRS}
                     ${Qt53DCore_INCLUDE_DIRS}
                     ${Qt53DRender_INCLUDE_DIRS}
                     ${Qt53DExtras_INCLUDE_DIRS}
                     ${libpyside_SOURCE_DIR}
                     ${QtCore_GEN_DIR}
                     ${QtGui_GEN_DIR}
                     ${Qt3DCore_GEN_DIR}
                     ${Qt3DRender_GEN_DIR})

set(Qt3DExtras_libraries pyside2
                     ${Qt53DExtras_LIBRARIES})

set(Qt3DExtras_deps Qt3DRender)

create_pyside_module(NAME Qt3DExtras
                     INCLUDE_DIRS Qt3DExtras_include_dirs
                     LIBRARIES Qt3DExtras_libraries
                     DEPS Qt3DExtras_deps
                     TYPESYSTEM_PATH Qt3DExtras_SOURCE_DIR
                     SOURCES Qt3DExtras_SRC
                     TYPESYSTEM_NAME ${Qt3DExtras_BINARY_DIR}/typesystem_3dextras.xml)
