project(QtMultimedia)

set(QtMultimedia_SRC
${QtMultimedia_GEN_DIR}/qabstractaudiodeviceinfo_wrapper.cpp
${QtMultimedia_GEN_DIR}/qabstractaudioinput_wrapper.cpp
${QtMultimedia_GEN_DIR}/qabstractaudiooutput_wrapper.cpp
${QtMultimedia_GEN_DIR}/qabstractvideobuffer_wrapper.cpp
${QtMultimedia_GEN_DIR}/qabstractvideofilter_wrapper.cpp
${QtMultimedia_GEN_DIR}/qabstractvideosurface_wrapper.cpp
${QtMultimedia_GEN_DIR}/qaudiobuffer_wrapper.cpp
${QtMultimedia_GEN_DIR}/qaudiodecoder_wrapper.cpp
${QtMultimedia_GEN_DIR}/qaudiodecodercontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qaudiodeviceinfo_wrapper.cpp
${QtMultimedia_GEN_DIR}/qaudioencodersettingscontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qaudioencodersettings_wrapper.cpp
${QtMultimedia_GEN_DIR}/qaudioformat_wrapper.cpp
${QtMultimedia_GEN_DIR}/qaudioinputselectorcontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qaudioinput_wrapper.cpp
${QtMultimedia_GEN_DIR}/qaudiooutputselectorcontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qaudiooutput_wrapper.cpp
${QtMultimedia_GEN_DIR}/qaudioprobe_wrapper.cpp
${QtMultimedia_GEN_DIR}/qaudiorecorder_wrapper.cpp
${QtMultimedia_GEN_DIR}/qaudiorolecontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qaudio_wrapper.cpp
${QtMultimedia_GEN_DIR}/qcameracapturebufferformatcontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qcameracapturedestinationcontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qcameracontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qcameraexposurecontrol_wrapper.cpp
# Private destructor: ${QtMultimedia_GEN_DIR}/qcameraexposure_wrapper.cpp
# Private destructor: ${${QtMultimedia_GEN_DIR}/qcamerafocus_wrapper.cpp
${QtMultimedia_GEN_DIR}/qcamerafocuszone_wrapper.cpp
${QtMultimedia_GEN_DIR}/qcamera_frameraterange_wrapper.cpp
${QtMultimedia_GEN_DIR}/qcameraimagecapturecontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qcameraimagecapture_wrapper.cpp
# Private destructor: ${QtMultimedia_GEN_DIR}/qcameraimageprocessing_wrapper.cpp
${QtMultimedia_GEN_DIR}/qcameraimageprocessingcontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qcamerainfocontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qcamerainfo_wrapper.cpp
${QtMultimedia_GEN_DIR}/qcameralockscontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qcameraviewfindersettingscontrol2_wrapper.cpp
${QtMultimedia_GEN_DIR}/qcameraviewfindersettingscontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qcameraviewfindersettings_wrapper.cpp
${QtMultimedia_GEN_DIR}/qcamera_wrapper.cpp
${QtMultimedia_GEN_DIR}/qcamerazoomcontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qimageencodercontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qimageencodersettings_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmediaaudioprobecontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmediaavailabilitycontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmediabindableinterface_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmediacontainercontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmediacontent_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmediacontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmediagaplessplaybackcontrol_wrapper.cpp
# Causes compile errors: ${QtMultimedia_GEN_DIR}/qmediametadata_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmedianetworkaccesscontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmediaobject_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmediaplayercontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmediaplayer_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmediaplaylist_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmediarecordercontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmediarecorder_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmediaresource_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmediatimerange_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmultimedia_wrapper.cpp
${QtMultimedia_GEN_DIR}/qradiodatacontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qradiodata_wrapper.cpp
${QtMultimedia_GEN_DIR}/qradiotunercontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qradiotuner_wrapper.cpp
${QtMultimedia_GEN_DIR}/qsoundeffect_wrapper.cpp
${QtMultimedia_GEN_DIR}/qsound_wrapper.cpp
${QtMultimedia_GEN_DIR}/qvideodeviceselectorcontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qvideoencodersettingscontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qvideoencodersettings_wrapper.cpp
${QtMultimedia_GEN_DIR}/qvideofilterrunnable_wrapper.cpp
${QtMultimedia_GEN_DIR}/qvideoframe_wrapper.cpp
${QtMultimedia_GEN_DIR}/qvideoprobe_wrapper.cpp
${QtMultimedia_GEN_DIR}/qvideorenderercontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qvideosurfaceformat_wrapper.cpp
${QtMultimedia_GEN_DIR}/qvideowindowcontrol_wrapper.cpp

# module is always needed
${QtMultimedia_GEN_DIR}/qtmultimedia_module_wrapper.cpp
)

make_path(QtMultimedia_typesystem_path  ${QtCore_SOURCE_DIR} ${QtGui_SOURCE_DIR} ${QtNetwork_SOURCE_DIR}
                                        ${QtCore_BINARY_DIR} ${QtGui_BINARY_DIR} ${QtNetwork_BINARY_DIR}
                                        ${QtMultimedia_SOURCE_DIR})

set(QtMultimedia_include_dirs   ${QtMultimedia_SOURCE_DIR}
                                ${QtMultimedia_BINARY_DIR}
                                ${Qt5Core_INCLUDE_DIRS}
                                ${Qt5Gui_INCLUDE_DIRS}
                                ${Qt5Network_INCLUDE_DIRS}
                                ${Qt5Multimedia_INCLUDE_DIRS}
                                ${SHIBOKEN_INCLUDE_DIR}
                                ${libpyside_SOURCE_DIR}
                                ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                                ${QtCore_GEN_DIR}
                                ${QtGui_GEN_DIR}
                                ${QtNetwork_GEN_DIR})

set(QtMultimedia_libraries  pyside2
                            ${SHIBOKEN_PYTHON_LIBRARIES}
                            ${SHIBOKEN_LIBRARY}
                            ${Qt5Multimedia_LIBRARIES}
                            ${Qt5Core_LIBRARIES}
                            ${Qt5Gui_LIBRARIES}
                            ${Qt5Network_LIBRARIES}
                            )
set(QtMultimedia_deps QtCore QtGui QtNetwork)

create_pyside_module(QtMultimedia
                     QtMultimedia_include_dirs
                     QtMultimedia_libraries
                     QtMultimedia_deps
                     QtMultimedia_typesystem_path
                     QtMultimedia_SRC
                     "")
