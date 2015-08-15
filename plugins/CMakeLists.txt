project(plugins)

set(ui_plugin_src
    customwidgets.cpp
    customwidget.cpp
)

set(ui_plugin_moc
    customwidget.h
    customwidgets.h
)

include_directories(${Qt5Designer_INCLUDE_DIRS}
                    ${SHIBOKEN_INCLUDE_DIR}
                    ${SHIBOKEN_PYTHON_INCLUDE_DIR})

qt5_wrap_cpp(MOC_FILES ${ui_plugin_moc})
add_library(uiplugin STATIC ${ui_plugin_src} ${MOC_FILES})
if(CMAKE_HOST_UNIX AND NOT CYGWIN)
    add_definitions(-fPIC)
endif()
add_definitions(-DQT_STATICPLUGIN)
target_link_libraries(uiplugin
                      ${Qt5UiTools_LIBRARY}
                      ${SHIBOKEN_LIBRARY}
                      ${SHIBOKEN_PYTHON_LIBRARY})
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LIBRARY_OUTPUT_SUFFIX ${CMAKE_DEBUG_POSTFIX})
else()
    set(LIBRARY_OUTPUT_SUFFIX ${CMAKE_RELEASE_POSTFIX})
endif()
