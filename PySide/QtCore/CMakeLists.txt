project(QtCore)

set(QtCore_gluecode "${CMAKE_CURRENT_SOURCE_DIR}/glue/qeasingcurve_glue.cpp")

if (${QT_VERSION_MAJOR} EQUAL 4 AND ${QT_VERSION_MINOR} LESS 6)
    set (QtCore_46_SRC )
else()
    set(QtCore_46_SRC
#  works       ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qeasingcurve_wrapper.cpp
###    ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qabstractanimation_wrapper.cpp
### metaObject problem     ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qabstractstate_wrapper.cpp
### metaObject problem     ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qabstracttransition_wrapper.cpp
### metaObject problem     ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qanimationgroup_wrapper.cpp
### metaObject problem     ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qeventtransition_wrapper.cpp
### metaObject problem     ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qfinalstate_wrapper.cpp
### metaObject problem     ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qhistorystate_wrapper.cpp
#  works       ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qmargins_wrapper.cpp
### metaObject problem     ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qparallelanimationgroup_wrapper.cpp
### metaObject problem     ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qpauseanimation_wrapper.cpp
#  works       ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qprocessenvironment_wrapper.cpp
### metaObject problem     ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qpropertyanimation_wrapper.cpp
### metaObject problem     ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qsequentialanimationgroup_wrapper.cpp
### metaObject problem     ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qsignaltransition_wrapper.cpp
### metaObject problem     ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qstate_wrapper.cpp
### metaObject problem     ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qstatemachine_wrapper.cpp
#  works       ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qstatemachine_signalevent_wrapper.cpp
#  works       ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qstatemachine_wrappedevent_wrapper.cpp
### metaObject problem     ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qvariantanimation_wrapper.cpp
    )
endif()
if (${QT_VERSION_MAJOR} EQUAL 4 AND ${QT_VERSION_MINOR} LESS 7)
    set (QtCore_47_SRC )
else()
    set(QtCore_47_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/qelapsedtimer_wrapper.cpp
    )
endif()

if(ENABLE_WIN)
    set(SPECIFIC_OS_FILES
        ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/msg_wrapper.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/point_wrapper.cpp
    )
else()
    set(SPECIFIC_OS_FILES "")
endif()

set(QtCore_SRC
### creates error ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qabstracteventdispatcher_wrapper.cpp
### creates error ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qabstractitemmodel_wrapper.cpp
### creates error ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qabstractlistmodel_wrapper.cpp
### creates error ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qabstracttablemodel_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qbasictimer_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qbitarray_wrapper.cpp
### creates error ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qbuffer_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qbytearray_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qbytearraymatcher_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qchildevent_wrapper.cpp
### creates error ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qcoreapplication_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qcryptographichash_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qdatastream_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qdate_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qdatetime_wrapper.cpp
### creates QFlag error ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qdir_wrapper.cpp
### creates QFlag error ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qdiriterator_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qdynamicpropertychangeevent_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qevent_wrapper.cpp
### creates QFlag error ### creates QFlag error ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qeventloop_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qfactoryinterface_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qfile_wrapper.cpp
### missing SBK_QFLAGS_QFILE_PERMISSION__IDX ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qfileinfo_wrapper.cpp
### metaObject problem ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qfilesystemwatcher_wrapper.cpp
### ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qfsfileengine_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qgenericargument_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qgenericreturnargument_wrapper.cpp
### QFlag problem ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qiodevice_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qlibraryinfo_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qline_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qlinef_wrapper.cpp
### QFlag problem ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qlocale_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmetaclassinfo_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmetaenum_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmetaproperty_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmetamethod_wrapper.cpp
#  works   
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmetaobject_wrapper.cpp
### metaObject problem ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmimedata_wrapper.cpp
### creates error ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmodelindex_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmutex_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qmutexlocker_wrapper.cpp
### metaObject problem 
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qobject_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qpersistentmodelindex_wrapper.cpp
### metaObject problem ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qpluginloader_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qpoint_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qpointf_wrapper.cpp
### metaObject problem ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qprocess_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qreadlocker_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qreadwritelock_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qrect_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qrectf_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qregexp_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qresource_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qrunnable_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qsemaphore_wrapper.cpp
### metaObject problem ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qsettings_wrapper.cpp
### metaObject problem ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qsignalmapper_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qsize_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qsizef_wrapper.cpp
### metaObject problem ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qsocketnotifier_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qsysinfo_wrapper.cpp
### no more public ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qsystemlocale_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qsystemsemaphore_wrapper.cpp
### QFlag problem ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qt_wrapper.cpp
### renamed, see QtConcurrent module ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtconcurrent_wrapper.cpp
### QBool, qInstallMsgHandler
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtcore_module_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtemporaryfile_wrapper.cpp
### QFlag problem ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtextboundaryfinder_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtextcodec_converterstate_wrapper.cpp
### QFlag problem ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtextcodec_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtextdecoder_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtextencoder_wrapper.cpp
### QFlag problem ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtextstream_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtextstreammanipulator_wrapper.cpp
### metaObject problem ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qthread_wrapper.cpp
### metaObject problem ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qthreadpool_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtime_wrapper.cpp
### metaObject problem ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtimeline_wrapper.cpp
### metaObject problem ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtimer_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtimerevent_wrapper.cpp
### metaObject problem ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qtranslator_wrapper.cpp
### QUrl changes ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qurl_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/quuid_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qwaitcondition_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qwritelocker_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qxmlstreamattribute_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qxmlstreamattributes_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qxmlstreamentitydeclaration_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qxmlstreamentityresolver_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qxmlstreamnamespacedeclaration_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qxmlstreamnotationdeclaration_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qxmlstreamreader_wrapper.cpp
#  works   ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtCore/qxmlstreamwriter_wrapper.cpp
${QtCore_46_SRC}
${QtCore_47_SRC}
${SPECIFIC_OS_FILES}
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/typesystem_core.xml.in"
               "${CMAKE_CURRENT_BINARY_DIR}/typesystem_core.xml" @ONLY)

set(prev_shiboken_binary ${SHIBOKEN_BINARY})
string(REPLACE ";" " " QtCore_SRC_escape "${QtCore_SRC}")
file(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/fake_shiboken.sh"
     "
echo \"$*\"
${prev_shiboken_binary} $*
python3 ${CMAKE_CURRENT_BINARY_DIR}/filter_init.py "${QtCore_SRC_escape}"
     ")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/filter_init.py"
     "#! env python
import sys, os, re, pprint
pprint.pprint(sys.argv)
nset = set()
for fname in sys.argv[1:]:
    name = os.path.splitext(os.path.basename(fname))[0]
    print(name)
    if name.endswith('module_wrapper'):
        fn = fname
    else:
        name = name.split('_wrapper')
        assert name[1] == ''
        nset.add(name[0])
print(fn)
with open(fn) as f:
    lines = f.readlines()
removals = []
for idx, line in enumerate(lines):
    res = re.search(' init_(\\w+)', line)
    if res and res.group(1).lower() not in nset:
        removals.append(res.group(1))
        lines[idx] = '//' + line
with open(fn, 'w') as f:
    f.writelines(lines)
print('Removals:', removals)
")
file(COPY ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/fake_shiboken.sh
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
  FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
  GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

set(SHIBOKEN_BINARY "${CMAKE_CURRENT_BINARY_DIR}/fake_shiboken.sh")

set(QtCore_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
# fake this variable for qt5


set(QtCore_typesystem_path "${QtCore_SOURCE_DIR}")

#message(FATAL_ERROR "${Qt5Core_INCLUDE_DIRS} ${testvar}")

set(QtCore_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_CURRENT_BINARY_DIR}/${BINDING_NAME}/QtCore/
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
