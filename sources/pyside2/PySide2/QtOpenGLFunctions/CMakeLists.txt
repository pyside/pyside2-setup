project(QtOpenGLFunctions)

set(QtOpenGLFunctions_DROPPED_ENTRIES)

get_property(QtOpenGLFunctions_enabled_features TARGET Qt${QT_MAJOR_VERSION}::Gui
             PROPERTY QT_ENABLED_PUBLIC_FEATURES)

set(QtOpenGLFunctions_SRC ${QtOpenGLFunctions_GEN_DIR}/qtopenglfunctions_module_wrapper.cpp)

# cf qtbase/src/gui/opengl/opengl.pri
list(FIND QtOpenGLFunctions_enabled_features "opengles2" _opengles2Index)
# ### fixme: For cmake >= 3.3: if(opengles2 IN_LIST QtOpenGLFunctions_enabled_features)
if(_opengles2Index GREATER -1)
    list(APPEND QtOpenGLFunctions_DROPPED_ENTRIES QOpenGLTimeMonitor QOpenGLTimerQuery)
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

set(QtOpenGLFunctions_include_dirs  ${QtGOpenGLFunctions_SOURCE_DIR}
                        ${QtOpenGLFunctions_BINARY_DIR}
                        ${pyside2_SOURCE_DIR}
                        ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                        ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                        ${libpyside_SOURCE_DIR}
                        ${QtCore_GEN_DIR}
                        ${QtGui_GEN_DIR}
                        )
set(QtOpenGLFunctions_libraries   pyside2
                      ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES})

set(QtOpenGLFunctions_deps QtGui)

create_pyside_module(NAME QtOpenGLFunctions
                     INCLUDE_DIRS QtOpenGLFunctions_include_dirs
                     LIBRARIES QtOpenGLFunctions_libraries
                     DEPS QtOpenGLFunctions_deps
                     TYPESYSTEM_PATH QtOpenGLFunctions_SOURCE_DIR
                     SOURCES QtOpenGLFunctions_SRC
                     TYPESYSTEM_NAME ${QtOpenGLFunctions_BINARY_DIR}/typesystem_openglfunctions.xml
                     DROPPED_ENTRIES QtOpenGLFunctions_DROPPED_ENTRIES)
