project(QtGui)

qt5_wrap_cpp(QPYTEXTOBJECT_MOC "${pyside2_SOURCE_DIR}/qpytextobject.h")

set(QtGui_DROPPED_ENTRIES)

get_property(QtGui_enabled_features TARGET Qt5::Gui PROPERTY INTERFACE_QT_ENABLED_FEATURES)

set(QtGui_SRC
${QtGui_GEN_DIR}/qabstractopenglfunctions_wrapper.cpp
${QtGui_GEN_DIR}/qabstracttextdocumentlayout_paintcontext_wrapper.cpp
${QtGui_GEN_DIR}/qabstracttextdocumentlayout_selection_wrapper.cpp
${QtGui_GEN_DIR}/qabstracttextdocumentlayout_wrapper.cpp
${QtGui_GEN_DIR}/qaccessible_wrapper.cpp
${QtGui_GEN_DIR}/qaccessibleeditabletextinterface_wrapper.cpp
${QtGui_GEN_DIR}/qaccessibleevent_wrapper.cpp
${QtGui_GEN_DIR}/qaccessibleobject_wrapper.cpp
${QtGui_GEN_DIR}/qaccessiblestatechangeevent_wrapper.cpp
${QtGui_GEN_DIR}/qaccessibletablecellinterface_wrapper.cpp
${QtGui_GEN_DIR}/qaccessibletablemodelchangeevent_wrapper.cpp
${QtGui_GEN_DIR}/qaccessibletextcursorevent_wrapper.cpp
${QtGui_GEN_DIR}/qaccessibletextinsertevent_wrapper.cpp
${QtGui_GEN_DIR}/qaccessibletextinterface_wrapper.cpp
${QtGui_GEN_DIR}/qaccessibletextremoveevent_wrapper.cpp
${QtGui_GEN_DIR}/qaccessibletextselectionevent_wrapper.cpp
${QtGui_GEN_DIR}/qaccessibletextupdateevent_wrapper.cpp
${QtGui_GEN_DIR}/qaccessiblevaluechangeevent_wrapper.cpp
${QtGui_GEN_DIR}/qaccessiblevalueinterface_wrapper.cpp
${QtGui_GEN_DIR}/qactionevent_wrapper.cpp
${QtGui_GEN_DIR}/qbackingstore_wrapper.cpp
${QtGui_GEN_DIR}/qbitmap_wrapper.cpp
${QtGui_GEN_DIR}/qbrush_wrapper.cpp
${QtGui_GEN_DIR}/qclipboard_wrapper.cpp
${QtGui_GEN_DIR}/qcloseevent_wrapper.cpp
${QtGui_GEN_DIR}/qcolor_wrapper.cpp
${QtGui_GEN_DIR}/qconicalgradient_wrapper.cpp
${QtGui_GEN_DIR}/qcontextmenuevent_wrapper.cpp
${QtGui_GEN_DIR}/qcursor_wrapper.cpp
${QtGui_GEN_DIR}/qdesktopservices_wrapper.cpp
${QtGui_GEN_DIR}/qdoublevalidator_wrapper.cpp
${QtGui_GEN_DIR}/qdrag_wrapper.cpp
${QtGui_GEN_DIR}/qdragenterevent_wrapper.cpp
${QtGui_GEN_DIR}/qdragleaveevent_wrapper.cpp
${QtGui_GEN_DIR}/qdragmoveevent_wrapper.cpp
${QtGui_GEN_DIR}/qdropevent_wrapper.cpp
${QtGui_GEN_DIR}/qenterevent_wrapper.cpp
${QtGui_GEN_DIR}/qexposeevent_wrapper.cpp
${QtGui_GEN_DIR}/qfileopenevent_wrapper.cpp
${QtGui_GEN_DIR}/qfocusevent_wrapper.cpp
${QtGui_GEN_DIR}/qfont_wrapper.cpp
${QtGui_GEN_DIR}/qfontdatabase_wrapper.cpp
${QtGui_GEN_DIR}/qfontinfo_wrapper.cpp
${QtGui_GEN_DIR}/qfontmetrics_wrapper.cpp
${QtGui_GEN_DIR}/qfontmetricsf_wrapper.cpp
${QtGui_GEN_DIR}/qgradient_wrapper.cpp
${QtGui_GEN_DIR}/qguiapplication_wrapper.cpp
${QtGui_GEN_DIR}/qhelpevent_wrapper.cpp
${QtGui_GEN_DIR}/qhideevent_wrapper.cpp
${QtGui_GEN_DIR}/qhoverevent_wrapper.cpp
${QtGui_GEN_DIR}/qicon_wrapper.cpp
${QtGui_GEN_DIR}/qicondragevent_wrapper.cpp
${QtGui_GEN_DIR}/qiconengine_availablesizesargument_wrapper.cpp
${QtGui_GEN_DIR}/qiconengine_wrapper.cpp
${QtGui_GEN_DIR}/qimage_wrapper.cpp
${QtGui_GEN_DIR}/qimageiohandler_wrapper.cpp
${QtGui_GEN_DIR}/qimagereader_wrapper.cpp
${QtGui_GEN_DIR}/qimagewriter_wrapper.cpp
${QtGui_GEN_DIR}/qinputevent_wrapper.cpp
${QtGui_GEN_DIR}/qinputmethod_wrapper.cpp
${QtGui_GEN_DIR}/qinputmethodevent_attribute_wrapper.cpp
${QtGui_GEN_DIR}/qinputmethodevent_wrapper.cpp
${QtGui_GEN_DIR}/qinputmethodqueryevent_wrapper.cpp
${QtGui_GEN_DIR}/qintvalidator_wrapper.cpp
${QtGui_GEN_DIR}/qkeyevent_wrapper.cpp
${QtGui_GEN_DIR}/qkeysequence_wrapper.cpp
${QtGui_GEN_DIR}/qlineargradient_wrapper.cpp
${QtGui_GEN_DIR}/qmatrix_wrapper.cpp
${QtGui_GEN_DIR}/qmatrix2x2_wrapper.cpp
${QtGui_GEN_DIR}/qmatrix2x3_wrapper.cpp
${QtGui_GEN_DIR}/qmatrix2x4_wrapper.cpp
${QtGui_GEN_DIR}/qmatrix3x2_wrapper.cpp
${QtGui_GEN_DIR}/qmatrix3x3_wrapper.cpp
${QtGui_GEN_DIR}/qmatrix3x4_wrapper.cpp
${QtGui_GEN_DIR}/qmatrix4x2_wrapper.cpp
${QtGui_GEN_DIR}/qmatrix4x3_wrapper.cpp
${QtGui_GEN_DIR}/qmatrix4x4_wrapper.cpp
${QtGui_GEN_DIR}/qmouseevent_wrapper.cpp
${QtGui_GEN_DIR}/qmoveevent_wrapper.cpp
${QtGui_GEN_DIR}/qmovie_wrapper.cpp
${QtGui_GEN_DIR}/qnativegestureevent_wrapper.cpp
${QtGui_GEN_DIR}/qoffscreensurface_wrapper.cpp
${QtGui_GEN_DIR}/qopenglcontextgroup_wrapper.cpp
${QtGui_GEN_DIR}/qopengldebuglogger_wrapper.cpp
${QtGui_GEN_DIR}/qopengldebugmessage_wrapper.cpp
${QtGui_GEN_DIR}/qopenglextrafunctions_wrapper.cpp
${QtGui_GEN_DIR}/qopenglframebufferobjectformat_wrapper.cpp
${QtGui_GEN_DIR}/qopenglfunctions_wrapper.cpp
# Compile error on Windows: ${QtGui_GEN_DIR}/qopenglpaintdevice_wrapper.cpp
${QtGui_GEN_DIR}/qopenglpixeltransferoptions_wrapper.cpp
${QtGui_GEN_DIR}/qopenglshaderprogram_wrapper.cpp
${QtGui_GEN_DIR}/qopengltexture_wrapper.cpp
${QtGui_GEN_DIR}/qopengltextureblitter_wrapper.cpp
${QtGui_GEN_DIR}/qopenglversionprofile_wrapper.cpp
${QtGui_GEN_DIR}/qopenglvertexarrayobject_wrapper.cpp
${QtGui_GEN_DIR}/qopenglvertexarrayobject_binder_wrapper.cpp
${QtGui_GEN_DIR}/qopenglwindow_wrapper.cpp
${QtGui_GEN_DIR}/qpagedpaintdevice_margins_wrapper.cpp
${QtGui_GEN_DIR}/qpagedpaintdevice_wrapper.cpp
${QtGui_GEN_DIR}/qpagelayout_wrapper.cpp
${QtGui_GEN_DIR}/qpagesize_wrapper.cpp
${QtGui_GEN_DIR}/qpaintdevice_wrapper.cpp
${QtGui_GEN_DIR}/qpaintdevicewindow_wrapper.cpp
${QtGui_GEN_DIR}/qpaintengine_wrapper.cpp
${QtGui_GEN_DIR}/qpaintenginestate_wrapper.cpp
${QtGui_GEN_DIR}/qpainter_pixmapfragment_wrapper.cpp
${QtGui_GEN_DIR}/qpainter_wrapper.cpp
${QtGui_GEN_DIR}/qpainterpath_element_wrapper.cpp
${QtGui_GEN_DIR}/qpainterpath_wrapper.cpp
${QtGui_GEN_DIR}/qpainterpathstroker_wrapper.cpp
${QtGui_GEN_DIR}/qpaintevent_wrapper.cpp
${QtGui_GEN_DIR}/qpalette_wrapper.cpp
${QtGui_GEN_DIR}/qpdfwriter_wrapper.cpp
${QtGui_GEN_DIR}/qpen_wrapper.cpp
${QtGui_GEN_DIR}/qpicture_wrapper.cpp
${QtGui_GEN_DIR}/qpictureio_wrapper.cpp
${QtGui_GEN_DIR}/qpixmap_wrapper.cpp
${QtGui_GEN_DIR}/qpixmapcache_key_wrapper.cpp
${QtGui_GEN_DIR}/qpixmapcache_wrapper.cpp
${QtGui_GEN_DIR}/qpixelformat_wrapper.cpp
${QtGui_GEN_DIR}/qpointingdeviceuniqueid_wrapper.cpp
${QtGui_GEN_DIR}/qpolygon_wrapper.cpp
${QtGui_GEN_DIR}/qpolygonf_wrapper.cpp
${QtGui_GEN_DIR}/qpytextobject_wrapper.cpp
${QtGui_GEN_DIR}/qquaternion_wrapper.cpp
${QtGui_GEN_DIR}/qradialgradient_wrapper.cpp
${QtGui_GEN_DIR}/qregexpvalidator_wrapper.cpp
${QtGui_GEN_DIR}/qregion_wrapper.cpp
${QtGui_GEN_DIR}/qresizeevent_wrapper.cpp
${QtGui_GEN_DIR}/qsessionmanager_wrapper.cpp
${QtGui_GEN_DIR}/qshortcutevent_wrapper.cpp
${QtGui_GEN_DIR}/qshowevent_wrapper.cpp
${QtGui_GEN_DIR}/qstandarditem_wrapper.cpp
${QtGui_GEN_DIR}/qstandarditemmodel_wrapper.cpp
${QtGui_GEN_DIR}/qstatustipevent_wrapper.cpp
${QtGui_GEN_DIR}/qopenglbuffer_wrapper.cpp
${QtGui_GEN_DIR}/qopenglcontext_wrapper.cpp
${QtGui_GEN_DIR}/qaccessible_state_wrapper.cpp
${QtGui_GEN_DIR}/qaccessibleinterface_wrapper.cpp
${QtGui_GEN_DIR}/qscreen_wrapper.cpp
${QtGui_GEN_DIR}/qopenglshader_wrapper.cpp
#${QtGui_GEN_DIR}/qopenglshaderprogram_wrapper.cpp
${QtGui_GEN_DIR}/qopenglframebufferobject_wrapper.cpp
${QtGui_GEN_DIR}/qrasterwindow_wrapper.cpp
${QtGui_GEN_DIR}/qrawfont_wrapper.cpp
${QtGui_GEN_DIR}/qscrollevent_wrapper.cpp
${QtGui_GEN_DIR}/qscrollprepareevent_wrapper.cpp
${QtGui_GEN_DIR}/qstatictext_wrapper.cpp
${QtGui_GEN_DIR}/qstylehints_wrapper.cpp
${QtGui_GEN_DIR}/qsurface_wrapper.cpp
${QtGui_GEN_DIR}/qsurfaceformat_wrapper.cpp
${QtGui_GEN_DIR}/qsyntaxhighlighter_wrapper.cpp
${QtGui_GEN_DIR}/qtabletevent_wrapper.cpp
${QtGui_GEN_DIR}/qtextblock_iterator_wrapper.cpp
${QtGui_GEN_DIR}/qtextblock_wrapper.cpp
${QtGui_GEN_DIR}/qtextblockformat_wrapper.cpp
${QtGui_GEN_DIR}/qtextblockgroup_wrapper.cpp
${QtGui_GEN_DIR}/qtextblockuserdata_wrapper.cpp
${QtGui_GEN_DIR}/qtextcharformat_wrapper.cpp
${QtGui_GEN_DIR}/qtextcursor_wrapper.cpp
${QtGui_GEN_DIR}/qtextdocument_wrapper.cpp
${QtGui_GEN_DIR}/qtextdocumentfragment_wrapper.cpp
${QtGui_GEN_DIR}/qtextdocumentwriter_wrapper.cpp
${QtGui_GEN_DIR}/qtextformat_wrapper.cpp
${QtGui_GEN_DIR}/qtextfragment_wrapper.cpp
${QtGui_GEN_DIR}/qtextframe_iterator_wrapper.cpp
${QtGui_GEN_DIR}/qtextframe_wrapper.cpp
${QtGui_GEN_DIR}/qtextframeformat_wrapper.cpp
${QtGui_GEN_DIR}/qtextimageformat_wrapper.cpp
${QtGui_GEN_DIR}/qtextinlineobject_wrapper.cpp
${QtGui_GEN_DIR}/qtextitem_wrapper.cpp
${QtGui_GEN_DIR}/qtextlayout_formatrange_wrapper.cpp
${QtGui_GEN_DIR}/qtextlayout_wrapper.cpp
${QtGui_GEN_DIR}/qtextlength_wrapper.cpp
${QtGui_GEN_DIR}/qtextline_wrapper.cpp
${QtGui_GEN_DIR}/qtextlist_wrapper.cpp
${QtGui_GEN_DIR}/qtextlistformat_wrapper.cpp
${QtGui_GEN_DIR}/qtextobject_wrapper.cpp
${QtGui_GEN_DIR}/qtextobjectinterface_wrapper.cpp
${QtGui_GEN_DIR}/qtextoption_tab_wrapper.cpp
${QtGui_GEN_DIR}/qtextoption_wrapper.cpp
${QtGui_GEN_DIR}/qtexttable_wrapper.cpp
${QtGui_GEN_DIR}/qtexttablecell_wrapper.cpp
${QtGui_GEN_DIR}/qtexttablecellformat_wrapper.cpp
${QtGui_GEN_DIR}/qtexttableformat_wrapper.cpp
${QtGui_GEN_DIR}/qtoolbarchangeevent_wrapper.cpp
${QtGui_GEN_DIR}/qtouchdevice_wrapper.cpp
${QtGui_GEN_DIR}/qtouchevent_touchpoint_wrapper.cpp
${QtGui_GEN_DIR}/qtouchevent_wrapper.cpp
${QtGui_GEN_DIR}/qtransform_wrapper.cpp
${QtGui_GEN_DIR}/qvalidator_wrapper.cpp
${QtGui_GEN_DIR}/qvector2d_wrapper.cpp
${QtGui_GEN_DIR}/qvector3d_wrapper.cpp
${QtGui_GEN_DIR}/qvector4d_wrapper.cpp
${QtGui_GEN_DIR}/qwhatsthisclickedevent_wrapper.cpp
${QtGui_GEN_DIR}/qwheelevent_wrapper.cpp
${QtGui_GEN_DIR}/qwindow_wrapper.cpp
${QtGui_GEN_DIR}/qwindowstatechangeevent_wrapper.cpp
# module is always needed
${QtGui_GEN_DIR}/qtgui_module_wrapper.cpp
)

# cf qtbase/src/gui/opengl/opengl.pri
list(FIND QtGui_enabled_features "opengles2" _opengles2Index)
# ### fixme: For cmake >= 3.3: if(opengles2 IN_LIST QtGui_enabled_features)
if(_opengles2Index GREATER -1)
    list(APPEND QtGui_DROPPED_ENTRIES QOpenGLTimeMonitor QOpenGLTimerQuery)
    message(STATUS "Qt5Gui: Dropping Desktop OpenGL classes (GLES2)")
else()
    list(APPEND QtGui_SRC
         ${QtGui_GEN_DIR}/qopengltimemonitor_wrapper.cpp
         ${QtGui_GEN_DIR}/qopengltimerquery_wrapper.cpp)
    message(STATUS "Qt5Gui: Adding Desktop OpenGL classes")
endif()

configure_file("${QtGui_SOURCE_DIR}/typesystem_gui.xml.in"
               "${QtGui_BINARY_DIR}/typesystem_gui.xml" @ONLY)

configure_file("${QtGui_SOURCE_DIR}/QtGui_global.post.h.in"
               "${QtGui_BINARY_DIR}/QtGui_global.post.h" @ONLY)

set(QtGui_include_dirs  ${QtGui_SOURCE_DIR}
                        ${QtGui_BINARY_DIR}
                        ${pyside2_SOURCE_DIR}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${Qt5Gui_INCLUDE_DIRS}
                        ${SHIBOKEN_INCLUDE_DIR}
                        ${libpyside_SOURCE_DIR}
                        ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                        ${QtCore_GEN_DIR}
                        )
set(QtGui_libraries   pyside2
                      ${SHIBOKEN_PYTHON_LIBRARIES}
                      ${SHIBOKEN_LIBRARY}
                      ${Qt5Gui_LIBRARIES})
set(QtGui_deps QtCore)

create_pyside_module(QtGui
                     QtGui_include_dirs
                     QtGui_libraries
                     QtGui_deps
                     QtGui_SOURCE_DIR
                     QtGui_SRC
                     QPYTEXTOBJECT_MOC
                     ${QtGui_BINARY_DIR}/typesystem_gui.xml
                     ""
                     ""
                     QtGui_DROPPED_ENTRIES)

install(FILES ${pyside2_SOURCE_DIR}/qpytextobject.h DESTINATION include/PySide2/QtGui/)

