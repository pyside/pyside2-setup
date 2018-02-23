project(libpyside)

if(${Qt5Qml_FOUND})
    if(NOT "${Qt5Qml_PRIVATE_INCLUDE_DIRS}" MATCHES "/QtQml/")
        string(REPLACE "/QtCore" "/QtQml" replaceme "${Qt5Core_PRIVATE_INCLUDE_DIRS}")
        list(APPEND Qt5Qml_PRIVATE_INCLUDE_DIRS ${replaceme})
        list(REMOVE_DUPLICATES Qt5Qml_PRIVATE_INCLUDE_DIRS)
    endif()
endif()

if(${Qt5Quick_FOUND})
    if(NOT "${Qt5Quick_PRIVATE_INCLUDE_DIRS}" MATCHES "/QtQuick/")
        string(REPLACE "/QtCore" "/QtQuick" replaceme "${Qt5Core_PRIVATE_INCLUDE_DIRS}")
        list(APPEND Qt5Quick_PRIVATE_INCLUDE_DIRS ${Qt5Qml_PRIVATE_INCLUDE_DIRS})
        list(APPEND Qt5Quick_PRIVATE_INCLUDE_DIRS ${replaceme})
        list(REMOVE_DUPLICATES Qt5Quick_PRIVATE_INCLUDE_DIRS)
    endif()
endif()

if(Qt5Qml_FOUND)
    # Used for registering custom QQuickItem classes defined in Python code.
    set(QML_SUPPORT 1)
    set(QML_INCLUDES ${Qt5Qml_INCLUDE_DIRS})
    set(QML_LIBRARIES ${Qt5Qml_LIBRARIES})

    if(Qt5Qml_PRIVATE_INCLUDE_DIRS)
        # Used for transforming QML exceptions into Python exceptions.
        set(QML_PRIVATE_API_SUPPORT 1)
        set(QML_INCLUDES ${QML_INCLUDES} ${Qt5Qml_PRIVATE_INCLUDE_DIRS})
    else()
        set(QML_PRIVATE_API_SUPPORT 0)
        message(WARNING "QML private API include files could not be found, support for catching QML exceptions inside Python code will not work.")
    endif()
else()
    set(QML_SUPPORT 0)
    set(QML_PRIVATE_API_SUPPORT 0)
    set(QML_INCLUDES "")
    set(QML_LIBRARIES "")
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

# Add python files to project explorer in Qt Creator, when opening the CMakeLists.txt as a project,
# so you can look up python files with the Locator.
macro(add_other_files)
     foreach(_it ${ARGN})
         if(NOT IS_DIRECTORY ${_it})
             get_filename_component(name ${_it} NAME)
             if(NOT ${_it} MATCHES "^/\\\\..*$;~$")
                 set_source_files_properties(${_it} PROPERTIES HEADER_FILE_ONLY TRUE)
             endif()
         endif()
     endforeach()
endmacro()

# Test files.
file(GLOB_RECURSE pyside_folder_py_files "../*.py")

# Mostly for setup.py.
file(GLOB setup_folder_py_files "../../../*.py")

set(other_files ${pyside_folder_py_files} ${setup_folder_py_files})
add_other_files(${other_files})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${SHIBOKEN_INCLUDE_DIR}
                    ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                    ${QML_INCLUDES}
                    ${Qt5Core_INCLUDE_DIRS})
add_library(pyside2 SHARED ${libpyside_SRC} ${other_files})
target_link_libraries(pyside2
                      ${SHIBOKEN_PYTHON_LIBRARIES}
                      ${SHIBOKEN_LIBRARY}
                      ${QML_LIBRARIES}
                      ${Qt5Core_LIBRARIES})

set_target_properties(pyside2 PROPERTIES
                      VERSION ${BINDING_API_VERSION}
                      SOVERSION "${PYSIDE_SO_VERSION}"
                      OUTPUT_NAME "pyside2${pyside2_SUFFIX}${SHIBOKEN_PYTHON_SHARED_LIBRARY_SUFFIX}"
                      DEFINE_SYMBOL PYSIDE_EXPORTS)

if(Qt5Core_VERSION VERSION_GREATER "5.7.1")
    set_property(TARGET pyside2 PROPERTY CXX_STANDARD 11)
endif()

if(QML_SUPPORT)
    target_compile_definitions(pyside2 PUBLIC PYSIDE_QML_SUPPORT=1)
endif()

if(PYSIDE_QT_CONF_PREFIX)
    set_property(SOURCE pyside.cpp
                 APPEND
                 PROPERTY COMPILE_DEFINITIONS
                 PYSIDE_QT_CONF_PREFIX=${PYSIDE_QT_CONF_PREFIX})
endif()

#
# install stuff
#

set(libpyside_HEADERS
    destroylistener.h
    dynamicqmetaobject.h
    globalreceiver.h
    pysideclassinfo.h
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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D QT_NO_CAST_FROM_ASCII -D QT_NO_CAST_TO_ASCII")

# create pkg-config file
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pyside2.pc.in"
               "${CMAKE_CURRENT_BINARY_DIR}/pyside2${pyside2_SUFFIX}.pc" @ONLY)
# create cmake-config files
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/PySide2Config.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/PySide2Config.cmake" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/PySide2Config-spec.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/PySide2Config${SHIBOKEN_PYTHON_CONFIG_SUFFIX}.cmake" @ONLY)
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
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PySide2Config${SHIBOKEN_PYTHON_CONFIG_SUFFIX}.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/PySide2-${BINDING_API_VERSION}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PySide2ConfigVersion.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/PySide2-${BINDING_API_VERSION}")
