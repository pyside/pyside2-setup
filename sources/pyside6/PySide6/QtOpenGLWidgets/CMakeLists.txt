project(QtOpenGLWidgets)

set(QtOpenGLWidgets_SRC
${QtOpenGLWidgets_GEN_DIR}/qopenglwidget_wrapper.cpp
# module is always needed
${QtOpenGLWidgets_GEN_DIR}/qtopenglwidgets_module_wrapper.cpp
)

set(QtOpenGLWidgets_include_dirs ${QtOpenGLWidgets_SOURCE_DIR}
                                 ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                                 ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                                 ${Qt${QT_MAJOR_VERSION}OpenGL_INCLUDE_DIRS}
                                 ${Qt${QT_MAJOR_VERSION}Widgets_INCLUDE_DIRS}
                                 ${libpyside_SOURCE_DIR}
                                 ${QtCore_GEN_DIR}
                                 ${QtGui_GEN_DIR}
                                 ${QtWidgets_GEN_DIR}
                                 ${QtOpenGLWidgets_GEN_DIR})

set(QtOpenGLWidgets_libraries pyside6
                              ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                              ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES}
                              ${Qt${QT_MAJOR_VERSION}OpenGL_LIBRARIES}
                              ${Qt${QT_MAJOR_VERSION}Widgets_LIBRARIES}
                              ${Qt${QT_MAJOR_VERSION}OpenGLWidgets_LIBRARIES})

set(QtOpenGLWidgets_deps QtOpenGL QtWidgets)

create_pyside_module(NAME QtOpenGLWidgets
                     INCLUDE_DIRS QtOpenGLWidgets_include_dirs
                     LIBRARIES QtOpenGLWidgets_libraries
                     DEPS QtOpenGLWidgets_deps
                     TYPESYSTEM_PATH QtOpenGLWidgets_SOURCE_DIR
                     SOURCES QtOpenGLWidgets_SRC)
