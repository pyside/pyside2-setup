project(Qt3DInput)

set(Qt3DInput_SRC
${Qt3DInput_GEN_DIR}/qt3dinput_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qabstractactioninput_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qabstractaxisinput_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qabstractphysicaldevice_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qaction_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qactioninput_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qanalogaxisinput_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qaxis_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qaxisaccumulator_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qaxissetting_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qbuttonaxisinput_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qinputaspect_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qinputchord_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qinputsequence_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qinputsettings_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qkeyboardhandler_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qkeyevent_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qlogicaldevice_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qkeyboarddevice_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qmousedevice_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qmouseevent_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qwheelevent_wrapper.cpp
${Qt3DInput_GEN_DIR}/qt3dinput_qmousehandler_wrapper.cpp
# module is always needed
${Qt3DInput_GEN_DIR}/qt3dinput_module_wrapper.cpp)

set(Qt3DInput_include_dirs
                     ${Qt3DInput_SOURCE_DIR}
                     ${Qt3DInput_BINARY_DIR}
                     ${pyside2_SOURCE_DIR}
                     ${Qt5Core_INCLUDE_DIRS}
                     ${SHIBOKEN_INCLUDE_DIR}
                     ${libpyside_SOURCE_DIR}
                     ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                     ${QtCore_GEN_DIR}
                     ${QtGui_GEN_DIR}
                     ${Qt3DCore_GEN_DIR})

set(Qt3DInput_libraries pyside2
                     ${SHIBOKEN_PYTHON_LIBRARIES}
                     ${SHIBOKEN_LIBRARY}
                     ${Qt53DInput_LIBRARIES})

set(Qt3DInput_deps Qt3DCore)

create_pyside_module(Qt3DInput
                     Qt3DInput_include_dirs
                     Qt3DInput_libraries
                     Qt3DInput_deps
                     Qt3DInput_SOURCE_DIR
                     Qt3DInput_SRC
                     ""
                     ${Qt3DInput_BINARY_DIR}/typesystem_3dinput.xml)
