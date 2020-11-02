project(Qt3DRender)

set(Qt3DRender_SRC
${Qt3DRender_GEN_DIR}/qt3drender_propertyreaderinterface_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qabstractlight_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qabstractraycaster_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qabstracttexture_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qabstracttextureimage_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qalphacoverage_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qalphatest_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qblendequation_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qblendequationarguments_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qblitframebuffer_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qbuffercapture_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qcamera_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qcameralens_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qcameraselector_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qclearbuffers_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qclipplane_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qcolormask_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qcomputecommand_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qcullface_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qdepthtest_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qdirectionallight_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qdispatchcompute_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qdithering_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qeffect_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qenvironmentlight_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qfilterkey_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qframegraphnode_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qfrontface_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qfrustumculling_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qgeometryrenderer_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qgraphicsapifilter_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qlayer_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qlayerfilter_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qlevelofdetail_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qlevelofdetailboundingsphere_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qlevelofdetailswitch_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qlinewidth_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qmaterial_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qmemorybarrier_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qmesh_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qmultisampleantialiasing_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qnodepthmask_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qnodraw_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qnopicking_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qobjectpicker_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qpaintedtextureimage_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qparameter_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qpickevent_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qpickingsettings_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qpicklineevent_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qpickpointevent_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qpicktriangleevent_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qpointlight_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qpointsize_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qpolygonoffset_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qproximityfilter_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qraycaster_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qraycasterhit_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qrenderaspect_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qrendercapabilities_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qrendercapture_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qrendercapturereply_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qrenderpass_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qrenderpassfilter_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qrendersettings_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qrenderstate_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qrenderstateset_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qrendersurfaceselector_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qrendertarget_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qrendertargetoutput_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qrendertargetselector_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qsceneloader_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qscissortest_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qscreenraycaster_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qseamlesscubemap_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qsetfence_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qshaderdata_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qshaderimage_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qshaderprogram_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qshaderprogrambuilder_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qsharedgltexture_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qsortpolicy_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qspotlight_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qstencilmask_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qstenciloperation_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qstenciloperationarguments_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qstenciltest_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qstenciltestarguments_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qtechnique_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qtechniquefilter_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qtexture1d_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qtexture1darray_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qtexture2d_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qtexture2darray_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qtexture2dmultisample_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qtexture2dmultisamplearray_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qtexture3d_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qtexturebuffer_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qtexturecubemap_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qtexturecubemaparray_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qtexturedata_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qtextureimage_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qtextureimagedata_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qtextureimagedatagenerator_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qtextureloader_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qtexturerectangle_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qtexturewrapmode_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qviewport_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_qwaitfence_wrapper.cpp
${Qt3DRender_GEN_DIR}/qt3drender_wrapper.cpp
# module is always needed
${Qt3DRender_GEN_DIR}/qt3drender_module_wrapper.cpp)

set(Qt3DRender_include_dirs
                     ${Qt3DRender_SOURCE_DIR}
                     ${Qt3DRender_BINARY_DIR}
                     ${pyside6_SOURCE_DIR}
                     ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                     ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                     ${libpyside_SOURCE_DIR}
                     ${QtCore_GEN_DIR}
                     ${QtGui_GEN_DIR}
                     ${QtOpenGL_GEN_DIR}
                     ${Qt3DCore_GEN_DIR})

set(Qt3DRender_libraries pyside6
                     ${Qt${QT_MAJOR_VERSION}3DRender_LIBRARIES})

set(Qt3DRender_deps Qt3DCore QtOpenGL)

create_pyside_module(NAME Qt3DRender
                     INCLUDE_DIRS Qt3DRender_include_dirs
                     LIBRARIES Qt3DRender_libraries
                     DEPS Qt3DRender_deps
                     TYPESYSTEM_PATH Qt3DRender_SOURCE_DIR
                     SOURCES Qt3DRender_SRC
                     TYPESYSTEM_NAME ${Qt3DRender_BINARY_DIR}/typesystem_3drender.xml)
