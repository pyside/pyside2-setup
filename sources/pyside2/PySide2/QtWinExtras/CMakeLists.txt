project(QtWinExtras)

set(QtWinExtras_SRC
${QtWinExtras_GEN_DIR}/qtwin_wrapper.cpp
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

set(QtWinExtras_include_dirs ${QtWinExtras_SOURCE_DIR}
                        ${QtWinExtras_BINARY_DIR}
                        ${Qt${QT_MAJOR_VERSION}WinExtras_INCLUDE_DIRS}
                        ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                        ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                        ${QtCore_GEN_DIR}
                        ${QtGui_GEN_DIR}
                        ${libpyside_SOURCE_DIR})

set(QtWinExtras_libraries pyside2
                        ${Qt${QT_MAJOR_VERSION}WinExtras_LIBRARIES}
                        ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                        ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES})

set(QtWinExtras_deps QtCore QtGui)

create_pyside_module(NAME QtWinExtras
                     INCLUDE_DIRS QtWinExtras_include_dirs
                     LIBRARIES QtWinExtras_libraries
                     DEPS QtWinExtras_deps
                     TYPESYSTEM_PATH QtWinExtras_SOURCE_DIR
                     SOURCES QtWinExtras_SRC)
