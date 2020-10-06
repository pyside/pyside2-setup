project(libpyside)

if(${Qt${QT_MAJOR_VERSION}Qml_FOUND})
    if(NOT "${Qt${QT_MAJOR_VERSION}Qml_PRIVATE_INCLUDE_DIRS}" MATCHES "/QtQml/")
        string(REPLACE "/QtCore" "/QtQml" replaceme "${Qt${QT_MAJOR_VERSION}Core_PRIVATE_INCLUDE_DIRS}")
        list(APPEND Qt${QT_MAJOR_VERSION}Qml_PRIVATE_INCLUDE_DIRS ${replaceme})
        list(REMOVE_DUPLICATES Qt${QT_MAJOR_VERSION}Qml_PRIVATE_INCLUDE_DIRS)
    endif()
endif()

if(${Qt${QT_MAJOR_VERSION}Quick_FOUND})
    if(NOT "${Qt${QT_MAJOR_VERSION}Quick_PRIVATE_INCLUDE_DIRS}" MATCHES "/QtQuick/")
        string(REPLACE "/QtCore" "/QtQuick" replaceme "${Qt${QT_MAJOR_VERSION}Core_PRIVATE_INCLUDE_DIRS}")
        list(APPEND Qt${QT_MAJOR_VERSION}Quick_PRIVATE_INCLUDE_DIRS ${Qt${QT_MAJOR_VERSION}Qml_PRIVATE_INCLUDE_DIRS})
        list(APPEND Qt${QT_MAJOR_VERSION}Quick_PRIVATE_INCLUDE_DIRS ${replaceme})
        list(REMOVE_DUPLICATES Qt${QT_MAJOR_VERSION}Quick_PRIVATE_INCLUDE_DIRS)
    endif()
endif()

set(QML_PRIVATE_API_SUPPORT 0)
if(Qt${QT_MAJOR_VERSION}Qml_FOUND)
    # Used for registering custom QQuickItem classes defined in Python code.
    set(QML_SUPPORT 1)
    set(QML_INCLUDES ${Qt${QT_MAJOR_VERSION}Qml_INCLUDE_DIRS})
    set(QML_LIBRARIES ${Qt${QT_MAJOR_VERSION}Qml_LIBRARIES})

    if(Qt${QT_MAJOR_VERSION}Qml_PRIVATE_INCLUDE_DIRS)
        # Used for transforming QML exceptions into Python exceptions.
        set(QML_PRIVATE_API_SUPPORT 1)
        set(QML_INCLUDES ${QML_INCLUDES} ${Qt${QT_MAJOR_VERSION}Qml_PRIVATE_INCLUDE_DIRS})
    else()
        message(WARNING "QML private API include files could not be found, support for catching QML exceptions inside Python code will not work.")
    endif()
else()
    set(QML_SUPPORT 0)
    set(QML_PRIVATE_API_SUPPORT 0)
    set(QML_INCLUDES "")
    set(QML_LIBRARIES "")
endif()

set(libpyside_SRC
    dynamicqmetaobject.cpp
    feature_select.cpp
    signalmanager.cpp
    globalreceiverv2.cpp
    pysideclassinfo.cpp
    pysideqenum.cpp
    pysidemetafunction.cpp
    pysidesignal.cpp
    pysideslot.cpp
    pysideproperty.cpp
    pysideqflags.cpp
    pysideweakref.cpp
    pyside.cpp
    pysidestaticstrings.cpp
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

add_library(pyside2 SHARED ${libpyside_SRC} ${other_files})
add_library(PySide2::pyside2 ALIAS pyside2)

target_include_directories(pyside2 PRIVATE
    ${QML_INCLUDES}
    ${Qt${QT_MAJOR_VERSION}Core_PRIVATE_INCLUDE_DIRS}
    ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
)

target_include_directories(pyside2 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/PySide2>
)

target_link_libraries(pyside2
                      PRIVATE Shiboken2::libshiboken
                      PRIVATE ${QML_LIBRARIES}
                      PRIVATE ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES})

set_target_properties(pyside2 PROPERTIES
                      VERSION ${BINDING_API_VERSION}
                      SOVERSION "${PYSIDE_SO_VERSION}"
                      OUTPUT_NAME "pyside2${pyside2_SUFFIX}${SHIBOKEN_PYTHON_SHARED_LIBRARY_SUFFIX}"
                      DEFINE_SYMBOL BUILD_LIBPYSIDE)

if(${QT_MAJOR_VERSION} GREATER_EQUAL 6)
    set_property(TARGET pyside2 PROPERTY CXX_STANDARD 17)
else()
    set_property(TARGET pyside2 PROPERTY CXX_STANDARD 11)
endif()

if(QML_SUPPORT)
    target_compile_definitions(pyside2 PUBLIC PYSIDE_QML_SUPPORT=1)
endif()
target_compile_definitions(pyside2 PRIVATE PYSIDE_QML_PRIVATE_API_SUPPORT=${QML_PRIVATE_API_SUPPORT})

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
    dynamicqmetaobject.h
    feature_select.h
    pysideclassinfo.h
    pysideqenum.h
    pysidemacros.h
    signalmanager.h
    pyside.h
    pysidestaticstrings.h
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

# for creating cmake-config files
include(CMakePackageConfigHelpers)

# Build-tree / super project package config file.
set(PYSIDE_PYTHONPATH "${pysidebindings_BINARY_DIR}/PySide2")
set(PYSIDE_TYPESYSTEMS "${pysidebindings_SOURCE_DIR}/PySide2/templates/")
set(PYSIDE_GLUE "${pysidebindings_SOURCE_DIR}/PySide2/glue")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/PySide2Config-spec.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/PySide2Config${SHIBOKEN_PYTHON_CONFIG_SUFFIX}.cmake"
     INSTALL_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
     PATH_VARS PYSIDE_PYTHONPATH PYSIDE_TYPESYSTEMS PYSIDE_GLUE
     INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
)

set(PYSIDE_PYTHONPATH "${PYTHON_SITE_PACKAGES}/PySide2")
set(PYSIDE_TYPESYSTEMS "${CMAKE_INSTALL_PREFIX}/share/PySide2${pyside2_SUFFIX}/typesystems")
set(PYSIDE_GLUE "${CMAKE_INSTALL_PREFIX}/share/PySide2${pyside2_SUFFIX}/glue")

# Install-tree / relocatable package config file.
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/PySide2Config-spec.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/install/PySide2Config${SHIBOKEN_PYTHON_CONFIG_SUFFIX}.cmake"
     INSTALL_DESTINATION "${LIB_INSTALL_DIR}/cmake/PySide2-${BINDING_API_VERSION}"
     PATH_VARS PYSIDE_PYTHONPATH PYSIDE_TYPESYSTEMS PYSIDE_GLUE
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/PySide2Config.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/PySide2Config.cmake" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/PySide2ConfigVersion.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/PySide2ConfigVersion.cmake" @ONLY)

install(FILES ${libpyside_HEADERS}
        DESTINATION include/${BINDING_NAME}${pyside2_SUFFIX})

install(TARGETS pyside2 EXPORT PySide2Targets
                        LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
                        ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
                        RUNTIME DESTINATION bin)
install(EXPORT PySide2Targets NAMESPACE PySide2::
        DESTINATION "${LIB_INSTALL_DIR}/cmake/PySide2-${BINDING_API_VERSION}")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/pyside2${pyside2_SUFFIX}.pc"
        DESTINATION "${LIB_INSTALL_DIR}/pkgconfig")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PySide2Config.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/PySide2-${BINDING_API_VERSION}")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/install/PySide2Config${SHIBOKEN_PYTHON_CONFIG_SUFFIX}.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/PySide2-${BINDING_API_VERSION}")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PySide2ConfigVersion.cmake"
        DESTINATION "${LIB_INSTALL_DIR}/cmake/PySide2-${BINDING_API_VERSION}")
