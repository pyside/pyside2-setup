project(QtOpenGL)

set(QtOpenGL_SRC
${QtOpenGL_GEN_DIR}/qgl_wrapper.cpp
${QtOpenGL_GEN_DIR}/qglbuffer_wrapper.cpp
${QtOpenGL_GEN_DIR}/qglcolormap_wrapper.cpp
${QtOpenGL_GEN_DIR}/qglcontext_wrapper.cpp
${QtOpenGL_GEN_DIR}/qglformat_wrapper.cpp
${QtOpenGL_GEN_DIR}/qglframebufferobject_wrapper.cpp
${QtOpenGL_GEN_DIR}/qglframebufferobjectformat_wrapper.cpp
${QtOpenGL_GEN_DIR}/qglpixelbuffer_wrapper.cpp
${QtOpenGL_GEN_DIR}/qglshader_wrapper.cpp
${QtOpenGL_GEN_DIR}/qglshaderprogram_wrapper.cpp
${QtOpenGL_GEN_DIR}/qglwidget_wrapper.cpp
# module is always needed
${QtOpenGL_GEN_DIR}/qtopengl_module_wrapper.cpp
)

set(QtOpenGL_include_dirs   ${QtOpenGL_SOURCE_DIR}
                            ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                            ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                            ${Qt${QT_MAJOR_VERSION}Widgets_INCLUDE_DIRS}
                            ${Qt${QT_MAJOR_VERSION}OpenGL_INCLUDE_DIRS}
                            ${libpyside_SOURCE_DIR}
                            ${QtWidgets_GEN_DIR}
                            ${QtGui_GEN_DIR}
                            ${QtCore_GEN_DIR}
                            ${QtOpenGL_GEN_DIR}
                            )
set(QtOpenGL_libraries      pyside2
                            ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                            ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES}
                            ${Qt${QT_MAJOR_VERSION}Widgets_LIBRARIES}
                            ${Qt${QT_MAJOR_VERSION}OpenGL_LIBRARIES})
set(QtOpenGL_deps QtWidgets)

create_pyside_module(NAME QtOpenGL
                     INCLUDE_DIRS QtOpenGL_include_dirs
                     LIBRARIES QtOpenGL_libraries
                     DEPS QtOpenGL_deps
                     TYPESYSTEM_PATH QtOpenGL_SOURCE_DIR
                     SOURCES QtOpenGL_SRC)
