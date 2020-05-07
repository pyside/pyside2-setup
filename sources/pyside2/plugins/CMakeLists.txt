project(plugins)

set(ui_plugin_src
    customwidgets.cpp
    customwidget.cpp
)

set(ui_plugin_moc
    customwidget.h
    customwidgets.h
)
include_directories(${Qt${QT_MAJOR_VERSION}Designer_INCLUDE_DIRS})

if (${QT_MAJOR_VERSION} GREATER_EQUAL 6)
    qt6_wrap_cpp(MOC_FILES ${ui_plugin_moc})
else()
    qt5_wrap_cpp(MOC_FILES ${ui_plugin_moc})
endif()

add_library(uiplugin STATIC ${ui_plugin_src} ${MOC_FILES})
if(CMAKE_HOST_UNIX AND NOT CYGWIN)
    add_definitions(-fPIC)
endif()
add_definitions(-DQT_STATICPLUGIN)

if(${QT_MAJOR_VERSION} GREATER_EQUAL 6)
    set_property(TARGET pyside2 PROPERTY CXX_STANDARD 17)
else()
    set_property(TARGET pyside2 PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(uiplugin
                      ${Qt${QT_MAJOR_VERSION}UiTools_LIBRARY}
                      Shiboken2::libshiboken)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LIBRARY_OUTPUT_SUFFIX ${CMAKE_DEBUG_POSTFIX})
else()
    set(LIBRARY_OUTPUT_SUFFIX ${CMAKE_RELEASE_POSTFIX})
endif()
