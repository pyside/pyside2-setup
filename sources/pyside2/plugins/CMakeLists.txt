project(plugins)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Widgets)
find_package(Qt6 COMPONENTS UiPlugin)

set(ui_plugin_src
    customwidgets.cpp
    customwidget.cpp
)

qt_wrap_cpp(MOC_FILES ${ui_plugin_moc})

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
                      Qt::Core
                      Qt::Gui
                      Qt::UiPlugin
                      Qt::Widgets
                      Shiboken6::libshiboken)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LIBRARY_OUTPUT_SUFFIX ${CMAKE_DEBUG_POSTFIX})
else()
    set(LIBRARY_OUTPUT_SUFFIX ${CMAKE_RELEASE_POSTFIX})
endif()
