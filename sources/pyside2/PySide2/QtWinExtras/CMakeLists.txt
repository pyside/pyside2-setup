project(QtWinExtras)

set(QtWinExtras_SRC
${QtWinExtras_GEN_DIR}/qwincolorizationchangeevent_wrapper.cpp
${QtWinExtras_GEN_DIR}/qwincompositionchangeevent_wrapper.cpp
${QtWinExtras_GEN_DIR}/qwinevent_wrapper.cpp
${QtWinExtras_GEN_DIR}/qwinjumplist_wrapper.cpp
${QtWinExtras_GEN_DIR}/qwinjumplistcategory_wrapper.cpp
${QtWinExtras_GEN_DIR}/qwinjumplistitem_wrapper.cpp
#${QtWinExtras_GEN_DIR}/qwinmime_wrapper.cpp
${QtWinExtras_GEN_DIR}/qwintaskbarbutton_wrapper.cpp
${QtWinExtras_GEN_DIR}/qwintaskbarprogress_wrapper.cpp
${QtWinExtras_GEN_DIR}/qwinthumbnailtoolbar_wrapper.cpp
${QtWinExtras_GEN_DIR}/qwinthumbnailtoolbutton_wrapper.cpp

# module is always needed
${QtWinExtras_GEN_DIR}/qtwinextras_module_wrapper.cpp
)

make_path(QtWinExtras_typesystem_path ${QtWinExtras_SOURCE_DIR}
                                      ${QtCore_SOURCE_DIR}
                                      ${QtCore_BINARY_DIR}
                                      ${QtGui_SOURCE_DIR}
                                      ${QtGui_BINARY_DIR})

set(QtWinExtras_include_dirs ${QtWinExtras_SOURCE_DIR}
                        ${QtWinExtras_BINARY_DIR}
                        ${Qt5WinExtras_INCLUDE_DIRS}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${Qt5Gui_INCLUDE_DIRS}
                        ${QtCore_GEN_DIR}
                        ${QtGui_GEN_DIR}
                        ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                        ${SHIBOKEN_INCLUDE_DIR}
                        ${libpyside_SOURCE_DIR})

set(QtWinExtras_libraries pyside2
                        ${Qt5WinExtras_LIBRARIES}
                        ${SHIBOKEN_PYTHON_LIBRARIES}
                        ${Qt5Core_LIBRARIES}
                        ${Qt5Gui_LIBRARIES})

set(QtWinExtras_deps QtCore QtGui)

create_pyside_module(QtWinExtras
                     QtWinExtras_include_dirs
                     QtWinExtras_libraries
                     QtWinExtras_deps
                     QtWinExtras_typesystem_path
                     QtWinExtras_SRC
                     "")
