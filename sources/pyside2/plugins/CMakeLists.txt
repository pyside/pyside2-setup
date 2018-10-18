project(plugins)

set(ui_plugin_src
    customwidgets.cpp
    customwidget.cpp
)

set(ui_plugin_moc
    customwidget.h
    customwidgets.h
)
include_directories(${Qt5Designer_INCLUDE_DIRS})

qt5_wrap_cpp(MOC_FILES ${ui_plugin_moc})
add_library(uiplugin STATIC ${ui_plugin_src} ${MOC_FILES})
if(CMAKE_HOST_UNIX AND NOT CYGWIN)
    add_definitions(-fPIC)
endif()
add_definitions(-DQT_STATICPLUGIN)

if(Qt5Core_VERSION VERSION_GREATER "5.7.1")
    set_property(TARGET uiplugin PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(uiplugin
                      ${Qt5UiTools_LIBRARY}
                      Shiboken2::libshiboken)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LIBRARY_OUTPUT_SUFFIX ${CMAKE_DEBUG_POSTFIX})
else()
    set(LIBRARY_OUTPUT_SUFFIX ${CMAKE_RELEASE_POSTFIX})
endif()
