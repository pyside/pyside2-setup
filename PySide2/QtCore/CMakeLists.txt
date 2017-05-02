project(QtCore)

set(QtCore_gluecode "${QtCore_SOURCE_DIR}/glue/qeasingcurve_glue.cpp")

if(ENABLE_WIN)
    set(SPECIFIC_OS_FILES
      ${QtCore_GEN_DIR}/qwineventnotifier_wrapper.cpp
    )
else()
    set(SPECIFIC_OS_FILES "")
endif()

# TODO: Fix cycling dependency  when generating QDataStream operator<< for QUuid
set (QtCore_SRC_FIXED_ME ${QtCore_GEN_DIR}/quuid_wrapper.cpp)

set(QtCore_SRC
${QtCore_GEN_DIR}/qabstractanimation_wrapper.cpp
${QtCore_GEN_DIR}/qabstracteventdispatcher_timerinfo_wrapper.cpp
${QtCore_GEN_DIR}/qabstracteventdispatcher_wrapper.cpp
${QtCore_GEN_DIR}/qabstractitemmodel_wrapper.cpp
${QtCore_GEN_DIR}/qabstractlistmodel_wrapper.cpp
${QtCore_GEN_DIR}/qabstractproxymodel_wrapper.cpp
${QtCore_GEN_DIR}/qabstractstate_wrapper.cpp
${QtCore_GEN_DIR}/qabstracttablemodel_wrapper.cpp
${QtCore_GEN_DIR}/qabstracttransition_wrapper.cpp
${QtCore_GEN_DIR}/qanimationgroup_wrapper.cpp
${QtCore_GEN_DIR}/qbasicmutex_wrapper.cpp
${QtCore_GEN_DIR}/qbasictimer_wrapper.cpp
${QtCore_GEN_DIR}/qbitarray_wrapper.cpp
${QtCore_GEN_DIR}/qbuffer_wrapper.cpp
${QtCore_GEN_DIR}/qbytearray_wrapper.cpp
${QtCore_GEN_DIR}/qbytearraymatcher_wrapper.cpp
${QtCore_GEN_DIR}/qchildevent_wrapper.cpp
${QtCore_GEN_DIR}/qcollator_wrapper.cpp
${QtCore_GEN_DIR}/qcollatorsortkey_wrapper.cpp
${QtCore_GEN_DIR}/qcommandlineoption_wrapper.cpp
${QtCore_GEN_DIR}/qcommandlineparser_wrapper.cpp
${QtCore_GEN_DIR}/qcoreapplication_wrapper.cpp
${QtCore_GEN_DIR}/qcryptographichash_wrapper.cpp
${QtCore_GEN_DIR}/qdatastream_wrapper.cpp
${QtCore_GEN_DIR}/qdate_wrapper.cpp
${QtCore_GEN_DIR}/qdatetime_wrapper.cpp
${QtCore_GEN_DIR}/qdir_wrapper.cpp
${QtCore_GEN_DIR}/qdiriterator_wrapper.cpp
${QtCore_GEN_DIR}/qdynamicpropertychangeevent_wrapper.cpp
${QtCore_GEN_DIR}/qeasingcurve_wrapper.cpp
${QtCore_GEN_DIR}/qelapsedtimer_wrapper.cpp
${QtCore_GEN_DIR}/qevent_wrapper.cpp
${QtCore_GEN_DIR}/qeventloop_wrapper.cpp
${QtCore_GEN_DIR}/qeventtransition_wrapper.cpp
${QtCore_GEN_DIR}/qfactoryinterface_wrapper.cpp
${QtCore_GEN_DIR}/qfile_wrapper.cpp
${QtCore_GEN_DIR}/qfileselector_wrapper.cpp
${QtCore_GEN_DIR}/qfiledevice_wrapper.cpp
${QtCore_GEN_DIR}/qfileinfo_wrapper.cpp
${QtCore_GEN_DIR}/qfilesystemwatcher_wrapper.cpp
${QtCore_GEN_DIR}/qfinalstate_wrapper.cpp
${QtCore_GEN_DIR}/qgenericargument_wrapper.cpp
${QtCore_GEN_DIR}/qgenericreturnargument_wrapper.cpp
${QtCore_GEN_DIR}/qhistorystate_wrapper.cpp
${QtCore_GEN_DIR}/qiodevice_wrapper.cpp
${QtCore_GEN_DIR}/qjsonarray_wrapper.cpp
${QtCore_GEN_DIR}/qjsondocument_wrapper.cpp
${QtCore_GEN_DIR}/qjsonparseerror_wrapper.cpp
${QtCore_GEN_DIR}/qjsonvalue_wrapper.cpp
${QtCore_GEN_DIR}/qitemselection_wrapper.cpp
${QtCore_GEN_DIR}/qitemselectionmodel_wrapper.cpp
${QtCore_GEN_DIR}/qitemselectionrange_wrapper.cpp
${QtCore_GEN_DIR}/qlibraryinfo_wrapper.cpp
${QtCore_GEN_DIR}/qline_wrapper.cpp
${QtCore_GEN_DIR}/qlinef_wrapper.cpp
${QtCore_GEN_DIR}/qlocale_wrapper.cpp
${QtCore_GEN_DIR}/qmargins_wrapper.cpp
${QtCore_GEN_DIR}/qmessagelogcontext_wrapper.cpp
${QtCore_GEN_DIR}/qmetaclassinfo_wrapper.cpp
${QtCore_GEN_DIR}/qmetaenum_wrapper.cpp
${QtCore_GEN_DIR}/qmetamethod_wrapper.cpp
${QtCore_GEN_DIR}/qmetaobject_connection_wrapper.cpp
${QtCore_GEN_DIR}/qmetaobject_wrapper.cpp
${QtCore_GEN_DIR}/qmetaproperty_wrapper.cpp
${QtCore_GEN_DIR}/qmimedata_wrapper.cpp
${QtCore_GEN_DIR}/qmimedatabase_wrapper.cpp
${QtCore_GEN_DIR}/qmimetype_wrapper.cpp
${QtCore_GEN_DIR}/qmodelindex_wrapper.cpp
${QtCore_GEN_DIR}/qmutex_wrapper.cpp
${QtCore_GEN_DIR}/qmutexlocker_wrapper.cpp
${QtCore_GEN_DIR}/qobject_wrapper.cpp
${QtCore_GEN_DIR}/qparallelanimationgroup_wrapper.cpp
${QtCore_GEN_DIR}/qpauseanimation_wrapper.cpp
${QtCore_GEN_DIR}/qpersistentmodelindex_wrapper.cpp
${QtCore_GEN_DIR}/qpluginloader_wrapper.cpp
${QtCore_GEN_DIR}/qpoint_wrapper.cpp
${QtCore_GEN_DIR}/qpointf_wrapper.cpp
${QtCore_GEN_DIR}/qprocess_wrapper.cpp
${QtCore_GEN_DIR}/qprocessenvironment_wrapper.cpp
${QtCore_GEN_DIR}/qpropertyanimation_wrapper.cpp
${QtCore_GEN_DIR}/qreadlocker_wrapper.cpp
${QtCore_GEN_DIR}/qreadwritelock_wrapper.cpp
${QtCore_GEN_DIR}/qrect_wrapper.cpp
${QtCore_GEN_DIR}/qrectf_wrapper.cpp
${QtCore_GEN_DIR}/qregexp_wrapper.cpp
${QtCore_GEN_DIR}/qresource_wrapper.cpp
${QtCore_GEN_DIR}/qrunnable_wrapper.cpp
${QtCore_GEN_DIR}/qsavefile_wrapper.cpp
${QtCore_GEN_DIR}/qsemaphore_wrapper.cpp
${QtCore_GEN_DIR}/qsequentialanimationgroup_wrapper.cpp
${QtCore_GEN_DIR}/qsettings_wrapper.cpp
${QtCore_GEN_DIR}/qsignalmapper_wrapper.cpp
${QtCore_GEN_DIR}/qsignaltransition_wrapper.cpp
${QtCore_GEN_DIR}/qsize_wrapper.cpp
${QtCore_GEN_DIR}/qsizef_wrapper.cpp
${QtCore_GEN_DIR}/qsocketnotifier_wrapper.cpp
${QtCore_GEN_DIR}/qsortfilterproxymodel_wrapper.cpp
${QtCore_GEN_DIR}/qstate_wrapper.cpp
${QtCore_GEN_DIR}/qstandardpaths_wrapper.cpp
${QtCore_GEN_DIR}/qstatemachine_signalevent_wrapper.cpp
${QtCore_GEN_DIR}/qstatemachine_wrappedevent_wrapper.cpp
${QtCore_GEN_DIR}/qstatemachine_wrapper.cpp
${QtCore_GEN_DIR}/qstorageinfo_wrapper.cpp
${QtCore_GEN_DIR}/qsysinfo_wrapper.cpp
${QtCore_GEN_DIR}/qsystemsemaphore_wrapper.cpp
${QtCore_GEN_DIR}/qt_wrapper.cpp
${QtCore_GEN_DIR}/qtemporarydir_wrapper.cpp
${QtCore_GEN_DIR}/qtemporaryfile_wrapper.cpp
${QtCore_GEN_DIR}/qtextboundaryfinder_wrapper.cpp
${QtCore_GEN_DIR}/qtextcodec_converterstate_wrapper.cpp
${QtCore_GEN_DIR}/qtextcodec_wrapper.cpp
${QtCore_GEN_DIR}/qtextdecoder_wrapper.cpp
${QtCore_GEN_DIR}/qtextencoder_wrapper.cpp
${QtCore_GEN_DIR}/qtextstream_wrapper.cpp
${QtCore_GEN_DIR}/qtextstreammanipulator_wrapper.cpp
${QtCore_GEN_DIR}/qthread_wrapper.cpp
${QtCore_GEN_DIR}/qthreadpool_wrapper.cpp
${QtCore_GEN_DIR}/qtime_wrapper.cpp
${QtCore_GEN_DIR}/qtimeline_wrapper.cpp
${QtCore_GEN_DIR}/qtimer_wrapper.cpp
${QtCore_GEN_DIR}/qtimerevent_wrapper.cpp
${QtCore_GEN_DIR}/qtranslator_wrapper.cpp
${QtCore_GEN_DIR}/qurl_wrapper.cpp
${QtCore_GEN_DIR}/qurlquery_wrapper.cpp
${QtCore_GEN_DIR}/qvariantanimation_wrapper.cpp
${QtCore_GEN_DIR}/qwaitcondition_wrapper.cpp
${QtCore_GEN_DIR}/qwritelocker_wrapper.cpp
${QtCore_GEN_DIR}/qxmlstreamattribute_wrapper.cpp
${QtCore_GEN_DIR}/qxmlstreamattributes_wrapper.cpp
${QtCore_GEN_DIR}/qxmlstreamentitydeclaration_wrapper.cpp
${QtCore_GEN_DIR}/qxmlstreamentityresolver_wrapper.cpp
${QtCore_GEN_DIR}/qxmlstreamnamespacedeclaration_wrapper.cpp
${QtCore_GEN_DIR}/qxmlstreamnotationdeclaration_wrapper.cpp
${QtCore_GEN_DIR}/qxmlstreamreader_wrapper.cpp
${QtCore_GEN_DIR}/qxmlstreamwriter_wrapper.cpp


${SPECIFIC_OS_FILES}
# module is always needed
${QtCore_GEN_DIR}/qtcore_module_wrapper.cpp
)

configure_file("${QtCore_SOURCE_DIR}/typesystem_core.xml.in"
               "${QtCore_BINARY_DIR}/typesystem_core.xml" @ONLY)

make_path(QtCore_typesystem_path ${QtCore_SOURCE_DIR})


set(QtCore_include_dirs ${QtCore_SOURCE_DIR}
                        ${QtCore_BINARY_DIR}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${SHIBOKEN_INCLUDE_DIR}
                        ${libpyside_SOURCE_DIR}
                        ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                        )
set(QtCore_libraries  pyside2
                      ${SHIBOKEN_PYTHON_LIBRARIES}
                      ${SHIBOKEN_LIBRARY}
                      ${Qt5Core_LIBRARIES}
                      )
create_pyside_module(QtCore
                     QtCore_include_dirs
                     QtCore_libraries
                     ""
                     QtCore_typesystem_path
                     QtCore_SRC
                     QtCore_gluecode
                     ${QtCore_BINARY_DIR}/typesystem_core.xml)

