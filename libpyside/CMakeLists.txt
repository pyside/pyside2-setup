project(libpyside)

if(Qt5Qml_FOUND AND Qt5Qml_PRIVATE_INCLUDE_DIRS)
    set(QML_INCLUDES ${Qt5Qml_INCLUDE_DIRS} ${Qt5Qml_PRIVATE_INCLUDE_DIRS})
    set(QML_LIBRARIES ${Qt5Qml_LIBRARIES})

    set(QML_SUPPORT 1)
else()
    set(QML_INCLUDES "")
    set(QML_LIBRARIES "")

    set(QML_SUPPORT 0)
endif()

qt5_wrap_cpp(DESTROYLISTENER_MOC "destroylistener.h")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/signalmanager.cpp.in"
               "${CMAKE_CURRENT_BINARY_DIR}/signalmanager.cpp" @ONLY)

set(libpyside_SRC
    dynamicqmetaobject.cpp
    destroylistener.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/signalmanager.cpp
    globalreceiver.cpp
    globalreceiverv2.cpp
    pysideclassinfo.cpp
    pysidemetafunction.cpp
    pysidesignal.cpp
    pysideslot.cpp
    pysideproperty.cpp
    pysideqflags.cpp
    pysideweakref.cpp
    pyside.cpp
    ${DESTROYLISTENER_MOC}
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${SHIBOKEN_INCLUDE_DIR}
                    ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                    ${QML_INCLUDES}
                    ${Qt5Core_INCLUDE_DIRS})
add_library(pyside2 SHARED ${libpyside_SRC})
target_link_libraries(pyside2
                      ${SHIBOKEN_PYTHON_LIBRARIES}
                      ${SHIBOKEN_LIBRARY}
                      ${QML_LIBRARIES}
                      ${Qt5Core_LIBRARIES})

set_target_properties(pyside2 PROPERTIES
                      VERSION ${BINDING_API_VERSION}
                      SOVERSION "${BINDING_API_MAJOR_VERSION}.${BINDING_API_MINOR_VERSION}"
                      OUTPUT_NAME "pyside2${pyside2_SUFFIX}${SHIBOKEN_PYTHON_EXTENSION_SUFFIX}"
                      DEFINE_SYMBOL PYSIDE_EXPORTS)

#
# install stuff
#

set(libpyside_HEADERS
    destroylistener.h
    dynamicqmetaobject.h
    globalreceiver.h
    pysideclassinfo.h
    pysideconversions.h
    pysidemacros.h
    signalmanager.h
    pyside.h
    pysidemetafunction.h
    pysidesignal.h
    pysideproperty.h
    pysideqflags.h
    pysideweakref.h
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LIBRARY_OUTPUT_SUFFIX ${CMAKE_DEBUG_POSTFIX})
else()
    set(LIBRARY_OUTPUT_SUFFIX ${CMAKE_RELEASE_POSTFIX})
endif()

# create pkg-config file
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pyside2.pc.in"
               "${CMAKE_CURRENT_BINARY_DIR}/pyside2${pyside2_SUFFIX}.pc" @ONLY)
# create cmake-config files
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/PySide2Config.cmake.in" 
               "${CMAKE_CURRENT_BINARY_DIR}/PySide2Config.cmake" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/PySide2Config-spec.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/PySide2Config${SHIBOKEN_PYTHON_EXTENSION_SUFFIX}.cmake" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/PySide2ConfigVersion.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/PySide2ConfigVersion.cmake" @ONLY)

install(FILES ${libpyside_HEADERS}
        DESTINATION include/${BINDING_NAME}${pyside2_SUFFIX})
install(TARGETS pyside2 EXPORT pyside2
                        LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
                        ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
                        RUNTIME DESTINATION bin)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/pyside2${pyside2_SUFFIX}.pc" 
        DESTINATION "${LIB_INSTALL_DIR}/pkgconfig")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PySide2Config.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/PySide2-${BINDING_API_VERSION}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PySide2Config${SHIBOKEN_PYTHON_EXTENSION_SUFFIX}.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/PySide2-${BINDING_API_VERSION}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PySide2ConfigVersion.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/PySide2-${BINDING_API_VERSION}")
