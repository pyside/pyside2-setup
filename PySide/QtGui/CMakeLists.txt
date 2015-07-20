project(QtGui)

if(ENABLE_X11)
    set(SPECIFIC_OS_FILES
        ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qx11info_wrapper.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qx11embedcontainer_wrapper.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qx11embedwidget_wrapper.cpp
    )

endif()

qt5_wrap_cpp(QPYTEXTOBJECT_MOC "${pyside_SOURCE_DIR}/qpytextobject.h")

set(QtGui_SRC
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qabstractproxymodel_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qabstracttextdocumentlayout_paintcontext_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qabstracttextdocumentlayout_selection_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qabstracttextdocumentlayout_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qaccessibleevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qactionevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qbitmap_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qbrush_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qclipboard_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qcloseevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qcolor_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qconicalgradient_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qcontextmenuevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qcursor_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qdoublevalidator_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qdrag_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qdragenterevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qdragleaveevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qdragmoveevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qdropevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qfileopenevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qfocusevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qfont_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qfontdatabase_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qfontinfo_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qfontmetrics_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qfontmetricsf_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qgradient_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qguiapplication_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qhelpevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qhideevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qhoverevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qicon_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qicondragevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qiconengine_availablesizesargument_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qiconengine_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qimage_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qimageiohandler_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qimagereader_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qimagewriter_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qinputevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qinputmethodevent_attribute_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qinputmethodevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qintvalidator_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qitemselection_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qitemselectionmodel_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qitemselectionrange_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qkeyevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qkeysequence_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qlineargradient_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qmatrix_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qmatrix2x2_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qmatrix2x3_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qmatrix2x4_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qmatrix3x2_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qmatrix3x3_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qmatrix3x4_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qmatrix4x2_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qmatrix4x3_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qmatrix4x4_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qmouseevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qmoveevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qmovie_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpagedpaintdevice_margins_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpagedpaintdevice_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpaintdevice_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpaintengine_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpaintenginestate_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpainter_pixmapfragment_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpainter_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpainterpath_element_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpainterpath_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpainterpathstroker_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpaintevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpalette_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpen_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpicture_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpictureio_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpixmap_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpixmapcache_key_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpixmapcache_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpolygon_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpolygonf_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qpytextobject_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qquaternion_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qradialgradient_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qregexpvalidator_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qregion_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qresizeevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qsessionmanager_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qshortcutevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qshowevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qsortfilterproxymodel_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qstandarditem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qstandarditemmodel_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qstatustipevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qstringlistmodel_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qsurface_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qsurfaceformat_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qsyntaxhighlighter_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtabletevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextblock_iterator_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextblock_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextblockformat_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextblockgroup_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextblockuserdata_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextcharformat_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextcursor_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextdocument_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextdocumentfragment_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextformat_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextfragment_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextframe_iterator_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextframe_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextframeformat_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextimageformat_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextinlineobject_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextlayout_formatrange_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextlayout_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextlength_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextline_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextlist_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextlistformat_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextobject_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextobjectinterface_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextoption_tab_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtextoption_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtexttable_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtexttablecell_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtexttablecellformat_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtexttableformat_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtoolbarchangeevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtouchevent_touchpoint_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtouchevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtransform_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qvalidator_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qvector2d_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qvector3d_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qvector4d_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qwhatsthisclickedevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qwheelevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qwindow_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qwindowstatechangeevent_wrapper.cpp

${SPECIFIC_OS_FILES}
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/qtgui_module_wrapper.cpp
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/typesystem_gui.xml.in"
               "${CMAKE_CURRENT_BINARY_DIR}/typesystem_gui.xml" @ONLY)

# fake these variables for qt5
set(QtGui_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(QtGui_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui)

make_path(QtGui_typesystem_path ${QtCore_SOURCE_DIR} ${QtCore_BINARY_DIR}
                                ${QtGui_SOURCE_DIR})

set(QtGui_include_dirs  ${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtGui/
                        ${pyside_SOURCE_DIR}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${Qt5Gui_INCLUDE_DIRS}
                        ${SHIBOKEN_INCLUDE_DIR}
                        ${libpyside_SOURCE_DIR}
                        ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                        ${QtCore_BINARY_DIR}/PySide/QtCore/)
set(QtGui_libraries   pyside
                      ${SHIBOKEN_PYTHON_LIBRARIES}
                      ${SHIBOKEN_LIBRARY}
                      ${Qt5Gui_LIBRARIES})
set(QtGui_deps "QtCore")

create_pyside_module(QtGui
                     QtGui_include_dirs
                     QtGui_libraries
                     QtGui_deps
                     QtGui_typesystem_path
                     QtGui_SRC
                     QPYTEXTOBJECT_MOC
                     ${CMAKE_CURRENT_BINARY_DIR}/typesystem_gui.xml)

install(FILES ${pyside_SOURCE_DIR}/qpytextobject.h DESTINATION include/PySide/QtGui/)
