project(QtOpenGLFunctions)

get_property(QtOpenGLFunctions_enabled_features TARGET Qt${QT_MAJOR_VERSION}::Gui
             PROPERTY QT_ENABLED_PUBLIC_FEATURES)

set(QtOpenGLFunctions_SRC ${QtOpenGLFunctions_GEN_DIR}/qtopenglfunctions_module_wrapper.cpp)

get_property(QtGui_enabled_features  TARGET Qt${QT_MAJOR_VERSION}::Gui
             PROPERTY QT_ENABLED_PUBLIC_FEATURES)

cmake_policy(SET CMP0057 NEW)
if("opengles2" IN_LIST QtGui_enabled_features)
    list(APPEND QtOpenGLFunctions_SRC
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_es2_wrapper.cpp)
    message(STATUS "Qt${QT_MAJOR_VERSION}OpenGLFunctions: Dropping Desktop OpenGL classes (GLES2)")
else()
    list(APPEND QtOpenGLFunctions_SRC
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_1_0_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_1_1_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_1_2_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_1_3_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_1_4_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_1_5_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_2_0_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_2_1_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_3_0_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_3_1_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_3_2_compatibility_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_3_2_core_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_3_3_compatibility_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_3_3_core_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_4_0_compatibility_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_4_0_core_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_4_1_compatibility_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_4_1_core_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_4_2_compatibility_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_4_2_core_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_4_3_compatibility_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_4_3_core_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_4_4_compatibility_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_4_4_core_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_4_5_compatibility_wrapper.cpp
         ${QtOpenGLFunctions_GEN_DIR}/qopenglfunctions_4_5_core_wrapper.cpp)
   message(STATUS "Qt${QT_MAJOR_VERSION}OpenGLFunctions: Adding Desktop OpenGL classes")
endif()

configure_file("${QtOpenGLFunctions_SOURCE_DIR}/QtOpenGLFunctions_global.post.h.in"
               "${QtOpenGLFunctions_BINARY_DIR}/QtOpenGLFunctions_global.post.h" @ONLY)

set(QtOpenGLFunctions_include_dirs ${QtOpenGLFunctions_SOURCE_DIR}
                                   ${QtOpenGLFunctions_BINARY_DIR}
                                   ${pyside2_SOURCE_DIR}
                                   ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                                   ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                                   ${Qt${QT_MAJOR_VERSION}OpenGL_INCLUDE_DIRS}
                                   ${libpyside_SOURCE_DIR}
                                   ${QtCore_GEN_DIR}
                                   ${QtGui_GEN_DIR}
                                   ${QtOpenGL_GEN_DIR}
                                   )

set(QtOpenGLFunctions_libraries pyside2
                                ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES}
                                ${Qt${QT_MAJOR_VERSION}OpenGL_LIBRARIES})

set(QtOpenGLFunctions_deps QtGui QtOpenGL)

create_pyside_module(NAME QtOpenGLFunctions
                     INCLUDE_DIRS QtOpenGLFunctions_include_dirs
                     LIBRARIES QtOpenGLFunctions_libraries
                     DEPS QtOpenGLFunctions_deps
                     TYPESYSTEM_PATH QtOpenGLFunctions_SOURCE_DIR
                     SOURCES QtOpenGLFunctions_SRC
                     TYPESYSTEM_NAME ${QtOpenGLFunctions_BINARY_DIR}/typesystem_openglfunctions.xml
                     DROPPED_ENTRIES QtOpenGLFunctions_DROPPED_ENTRIES)
