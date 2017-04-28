project(QtMultimedia)

set(QtMultimedia_SRC
${QtMultimedia_GEN_DIR}/qaudio_wrapper.cpp
${QtMultimedia_GEN_DIR}/qabstractvideobuffer_wrapper.cpp
${QtMultimedia_GEN_DIR}/qvideosurfaceformat_wrapper.cpp
${QtMultimedia_GEN_DIR}/qabstractvideosurface_wrapper.cpp
${QtMultimedia_GEN_DIR}/qaudiodeviceinfo_wrapper.cpp
${QtMultimedia_GEN_DIR}/qabstractaudiodeviceinfo_wrapper.cpp
${QtMultimedia_GEN_DIR}/qabstractaudiooutput_wrapper.cpp
${QtMultimedia_GEN_DIR}/qabstractaudioinput_wrapper.cpp
${QtMultimedia_GEN_DIR}/qaudioformat_wrapper.cpp
${QtMultimedia_GEN_DIR}/qaudioinput_wrapper.cpp
${QtMultimedia_GEN_DIR}/qaudiooutput_wrapper.cpp
${QtMultimedia_GEN_DIR}/qvideoframe_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmediabindableinterface_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmediacontrol_wrapper.cpp
${QtMultimedia_GEN_DIR}/qmediaobject_wrapper.cpp

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
                                ${QtNetwork_GEN_DIR}
                                )
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
