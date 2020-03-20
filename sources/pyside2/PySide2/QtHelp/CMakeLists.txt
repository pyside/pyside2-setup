project(QtHelp)

set(QtHelp_SRC
${QtHelp_GEN_DIR}/qhelpcontentitem_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpcontentmodel_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpcontentwidget_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpenginecore_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpengine_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpindexmodel_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpindexwidget_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpsearchengine_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpsearchquerywidget_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpsearchquery_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpsearchresult_wrapper.cpp
${QtHelp_GEN_DIR}/qhelpsearchresultwidget_wrapper.cpp
# module is always needed
${QtHelp_GEN_DIR}/qthelp_module_wrapper.cpp
)

if (Qt5Help_VERSION VERSION_EQUAL 5.13.0 OR Qt5Help_VERSION VERSION_GREATER 5.13.0)
    list(APPEND QtHelp_SRC
         ${QtHelp_GEN_DIR}/qcompressedhelpinfo_wrapper.cpp
         ${QtHelp_GEN_DIR}/qhelpfilterdata_wrapper.cpp
         ${QtHelp_GEN_DIR}/qhelpfilterengine_wrapper.cpp)
endif()

if (Qt5Help_VERSION VERSION_EQUAL 5.15.0 OR Qt5Help_VERSION VERSION_GREATER 5.15.0)
    list(APPEND QtHelp_SRC
         ${QtHelp_GEN_DIR}/qhelpfiltersettingswidget_wrapper.cpp
         ${QtHelp_GEN_DIR}/qhelplink_wrapper.cpp)
endif()


set(QtHelp_include_dirs ${QtHelp_SOURCE_DIR}
                        ${QtHelp_BINARY_DIR}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${Qt5Gui_INCLUDE_DIRS}
                        ${Qt5Widgets_INCLUDE_DIRS}
                        ${Qt5Help_INCLUDE_DIRS}
                        ${libpyside_SOURCE_DIR}
                        ${QtWidgets_GEN_DIR}
                        ${QtGui_GEN_DIR}
                        ${QtCore_GEN_DIR}
                        )
set(QtHelp_libraries  pyside2
                      ${Qt5Core_LIBRARIES}
                      ${Qt5Gui_LIBRARIES}
                      ${Qt5Widgets_LIBRARIES}
                      ${Qt5Help_LIBRARIES})

set(QtHelp_deps QtWidgets)

create_pyside_module(NAME QtHelp
                     INCLUDE_DIRS QtHelp_include_dirs
                     LIBRARIES QtHelp_libraries
                     DEPS QtHelp_deps
                     TYPESYSTEM_PATH QtHelp_SOURCE_DIR
                     SOURCES QtHelp_SRC)
