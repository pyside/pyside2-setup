project(QtCore)

set(QtCore_gluecode "${CMAKE_CURRENT_SOURCE_DIR}/glue/qeasingcurve_glue.cpp")

if(ENABLE_WIN)
    set(SPECIFIC_OS_FILES
        ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/msg_wrapper.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/point_wrapper.cpp
    )
else()
    set(SPECIFIC_OS_FILES "")
endif()

set(QtCore_SRC
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qabstractanimation_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qabstracteventdispatcher_timerinfo_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qabstracteventdispatcher_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qabstractitemmodel_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qabstractlistmodel_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qabstractstate_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qabstracttablemodel_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qabstracttransition_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qanimationgroup_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qbasicmutex_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qbasictimer_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qbitarray_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qbuffer_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qbytearray_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qbytearraymatcher_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qchildevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qcoreapplication_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qcryptographichash_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qdatastream_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qdate_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qdatetime_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qdir_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qdiriterator_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qdynamicpropertychangeevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qeasingcurve_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qelapsedtimer_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qeventloop_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qeventtransition_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qfactoryinterface_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qfile_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qfiledevice_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qfileinfo_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qfilesystemwatcher_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qfinalstate_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qgenericargument_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qgenericreturnargument_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qhistorystate_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qiodevice_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qlibraryinfo_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qline_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qlinef_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qlocale_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmargins_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmetaclassinfo_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmetaenum_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmetamethod_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmetaobject_connection_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmetaobject_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmetaproperty_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmimedata_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmodelindex_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmutex_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmutexlocker_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qobject_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qparallelanimationgroup_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qpauseanimation_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qpersistentmodelindex_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qpluginloader_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qpoint_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qpointf_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qprocess_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qprocessenvironment_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qpropertyanimation_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qreadlocker_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qreadwritelock_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qrect_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qrectf_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qregexp_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qresource_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qrunnable_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qsemaphore_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qsequentialanimationgroup_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qsettings_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qsignalmapper_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qsignaltransition_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qsize_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qsizef_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qsocketnotifier_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qstate_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qstatemachine_signalevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qstatemachine_wrappedevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qstatemachine_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qsysinfo_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qsystemsemaphore_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qt_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtemporaryfile_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtextboundaryfinder_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtextcodec_converterstate_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtextcodec_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtextdecoder_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtextencoder_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtextstream_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtextstreammanipulator_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qthread_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qthreadpool_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtime_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtimeline_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtimer_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtimerevent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtranslator_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qurl_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/quuid_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qvariantanimation_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qwaitcondition_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qwritelocker_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qxmlstreamattribute_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qxmlstreamattributes_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qxmlstreamentitydeclaration_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qxmlstreamentityresolver_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qxmlstreamnamespacedeclaration_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qxmlstreamnotationdeclaration_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qxmlstreamreader_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qxmlstreamwriter_wrapper.cpp

${SPECIFIC_OS_FILES}
# module is always needed
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtcore_module_wrapper.cpp
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/typesystem_core.xml.in"
               "${CMAKE_CURRENT_BINARY_DIR}/typesystem_core.xml" @ONLY)

make_path(QtCore_typesystem_path ${QtCore_SOURCE_DIR})

set(QtCore_include_dirs ${QtCore_SOURCE_DIR}
                        ${QtCore_BINARY_DIR}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${SHIBOKEN_INCLUDE_DIR}
                        ${libpyside_SOURCE_DIR}
                        ${SHIBOKEN_PYTHON_INCLUDE_DIR})
set(QtCore_libraries  pyside
                      ${SHIBOKEN_PYTHON_LIBRARIES}
                      ${SHIBOKEN_LIBRARY}
                      ${Qt5Core_LIBRARIES})

create_pyside_module(QtCore
                     QtCore_include_dirs
                     QtCore_libraries
                     ""
                     QtCore_typesystem_path
                     QtCore_SRC
                     QtCore_gluecode
                     ${CMAKE_CURRENT_BINARY_DIR}/typesystem_core.xml)
