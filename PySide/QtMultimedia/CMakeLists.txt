project(QtMultimedia)

set(QtMultimedia_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtMultimedia/qaudio_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtMultimedia/qabstractvideobuffer_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtMultimedia/qaudioenginefactoryinterface_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtMultimedia/qvideosurfaceformat_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtMultimedia/qabstractvideosurface_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtMultimedia/qaudiodeviceinfo_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtMultimedia/qabstractaudiodeviceinfo_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtMultimedia/qabstractaudiooutput_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtMultimedia/qabstractaudioinput_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtMultimedia/qaudioengineplugin_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtMultimedia/qaudioformat_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtMultimedia/qaudioinput_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtMultimedia/qaudiooutput_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtMultimedia/qvideoframe_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtMultimedia/qtmultimedia_module_wrapper.cpp
)
make_path(QtMultimedia_typesystem_path  ${QtCore_SOURCE_DIR} ${QtGui_SOURCE_DIR}
                                        ${QtCore_BINARY_DIR} ${QtGui_BINARY_DIR}
                                        ${QtMultimedia_SOURCE_DIR})

set(QtMultimedia_include_dirs   ${CMAKE_CURRENT_SOURCE_DIR}
                                ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtMultimedia/
                                ${Qt5Core_INCLUDE_DIRS}
                                ${Qt5Gui_INCLUDE_DIRS}
                                ${Qt5Multimedia_INCLUDE_DIRS}
                                ${SHIBOKEN_INCLUDE_DIR}
                                ${libpyside_SOURCE_DIR}
                                ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                                ${QtCore_BINARY_DIR}/PySide/QtCore/
                                ${QtGui_BINARY_DIR}/PySide/QtGui/)

set(QtMultimedia_libraries  pyside
                            ${SHIBOKEN_PYTHON_LIBRARIES}
                            ${SHIBOKEN_LIBRARY}
                            ${Qt5Multimedia_LIBRARIES}
                            ${Qt5Gui_LIBRARIES})

set(QtMultimedia_deps QtGui)

create_pyside_module(QtMultimedia
                     QtMultimedia_include_dirs
                     QtMultimedia_libraries
                     QtMultimedia_deps
                     QtMultimedia_typesystem_path
                     QtMultimedia_SRC
                     "")
