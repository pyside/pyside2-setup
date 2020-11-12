project(QtHelp)

set(QtHelp_SRC
${QtHelp_GEN_DIR}/qcompressedhelpinfo_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpcontentitem_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpcontentmodel_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpcontentwidget_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpengine_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpenginecore_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpfilterdata_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpfilterengine_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpfiltersettingswidget_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpindexmodel_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpindexwidget_wrapper.cpp
${QtHelp_GEN_DIR}/qhelplink_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpsearchengine_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpsearchquery_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpsearchquerywidget_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpsearchresult_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpsearchresultwidget_wrapper.cpp
# module is always needed
${QtHelp_GEN_DIR}/qthelp_module_wrapper.cpp
)

set(QtHelp_include_dirs ${QtHelp_SOURCE_DIR}
                        ${QtHelp_BINARY_DIR}
                        ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                        ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                        ${Qt${QT_MAJOR_VERSION}Widgets_INCLUDE_DIRS}
                        ${Qt${QT_MAJOR_VERSION}Help_INCLUDE_DIRS}
                        ${libpyside_SOURCE_DIR}
                        ${QtWidgets_GEN_DIR}
                        ${QtGui_GEN_DIR}
                        ${QtCore_GEN_DIR}
                        )
set(QtHelp_libraries  pyside6
                      ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                      ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES}
                      ${Qt${QT_MAJOR_VERSION}Widgets_LIBRARIES}
                      ${Qt${QT_MAJOR_VERSION}Help_LIBRARIES})

set(QtHelp_deps QtWidgets)

create_pyside_module(NAME QtHelp
                     INCLUDE_DIRS QtHelp_include_dirs
                     LIBRARIES QtHelp_libraries
                     DEPS QtHelp_deps
                     TYPESYSTEM_PATH QtHelp_SOURCE_DIR
                     SOURCES QtHelp_SRC)
