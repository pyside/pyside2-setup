project(pyside2)

include(PySideModules)

macro(COLLECT_MODULE_IF_FOUND shortname)
    set(name "Qt5${shortname}")
    find_package(${name})
    set(_name_found "${name}_FOUND")
    if(${_name_found})
        message(STATUS "module ${name} found (${ARGN})")

        # Hoist this to the parent scope to make sure all tests get built
        set("${name}_FOUND" 1 PARENT_SCOPE)
        # also put the module at the end of pyside2_global.h
        file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/pyside2_global.h.add"
                    "#include \"Qt${shortname}/Qt${shortname}\"\n")
        # record the shortnames for the tests
        list(APPEND all_module_shortnames ${shortname})
        set(all_module_shortnames ${all_module_shortnames} PARENT_SCOPE)
    else()
        if("${ARGN}" STREQUAL "opt")
            message(STATUS "optional module ${name} skipped")
        elseif("${ARGN}" STREQUAL "essential")
            message(STATUS "skipped optional module ${name} is essential!\n"
                           "   We do not guarantee that all tests are working.")
        else()
            message(FATAL_ERROR "module ${name} MISSING")
        endif()
    endif()
endmacro()

# note: the order of this list is relevant for dependencies.
# For instance: Qt5Printsupport must come before Qt5WebKitWidgets.
COLLECT_MODULE_IF_FOUND(Core)
COLLECT_MODULE_IF_FOUND(Gui essential)
COLLECT_MODULE_IF_FOUND(Widgets essential)
COLLECT_MODULE_IF_FOUND(PrintSupport essential)
COLLECT_MODULE_IF_FOUND(Sql essential)
COLLECT_MODULE_IF_FOUND(Network essential)
COLLECT_MODULE_IF_FOUND(Test essential)
if(NOT MSVC)
    # right now this does not build on windows
    COLLECT_MODULE_IF_FOUND(Concurrent essential)
else()
    # Note: in order to use this variable in a sibling project (tests),
    # we need to either make the value persistent like so:
    # set(DISABLE_QtConcurrent 1 CACHE INTERNAL "")
    # or export it to the caller's scope by the PARENT_SCOPE specifier.
    # This is preferable, because there are no sticky side effects.
    set(DISABLE_QtConcurrent 1 PARENT_SCOPE)
ENDIF()
if(UNIX AND NOT APPLE)
    COLLECT_MODULE_IF_FOUND(X11Extras essential)
endif()
if(WIN32)
    COLLECT_MODULE_IF_FOUND(WinExtras essential)
endif()
if(APPLE)
    COLLECT_MODULE_IF_FOUND(MacExtras essential)
endif()
COLLECT_MODULE_IF_FOUND(Xml)
COLLECT_MODULE_IF_FOUND(XmlPatterns opt)
COLLECT_MODULE_IF_FOUND(Help opt)
COLLECT_MODULE_IF_FOUND(Multimedia opt)
COLLECT_MODULE_IF_FOUND(MultimediaWidgets opt)
COLLECT_MODULE_IF_FOUND(OpenGL opt)
COLLECT_MODULE_IF_FOUND(Qml opt)
COLLECT_MODULE_IF_FOUND(Quick opt)
COLLECT_MODULE_IF_FOUND(QuickWidgets opt)
COLLECT_MODULE_IF_FOUND(Script opt)
if(NOT MSVC)
    # right now this does not build on windows
    COLLECT_MODULE_IF_FOUND(ScriptTools opt)
else()
    set(DISABLE_QtScriptTools 1 PARENT_SCOPE)
ENDIF()
COLLECT_MODULE_IF_FOUND(Svg opt)
if(Qt5Designer_FOUND)
    COLLECT_MODULE_IF_FOUND(UiTools opt)
else()
    set(DISABLE_QtUiTools 1 PARENT_SCOPE)
endif()
COLLECT_MODULE_IF_FOUND(WebChannel opt)
# still forgotten:
#COLLECT_MODULE_IF_FOUND(WebEngineCore opt)
#COLLECT_MODULE_IF_FOUND(WebEngine opt)
COLLECT_MODULE_IF_FOUND(WebEngineWidgets opt)
COLLECT_MODULE_IF_FOUND(WebKit opt)
if(NOT MSVC)
    # right now this does not build on windows
    COLLECT_MODULE_IF_FOUND(WebKitWidgets opt)
else()
    set(DISABLE_QtWebKitWidgets 1 PARENT_SCOPE)
ENDIF()
COLLECT_MODULE_IF_FOUND(WebSockets opt)

# Configure include based on platform
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/global.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/pyside2_global.h" @ONLY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
               "${CMAKE_CURRENT_BINARY_DIR}/__init__.py" @ONLY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/_utils.py.in"
               "${CMAKE_CURRENT_BINARY_DIR}/_utils.py" @ONLY)

# Add the collected includes to the global include
file(READ   "${CMAKE_CURRENT_BINARY_DIR}/pyside2_global.h.add" GLOBAL_APPEND)
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/pyside2_global.h" "${GLOBAL_APPEND}")
file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/pyside2_global.h.add")

# now compile all modules.
foreach(shortname IN LISTS all_module_shortnames)
    set(name "Qt5${shortname}")
    HAS_QT_MODULE(${name}_FOUND Qt${shortname})
endforeach()

# install
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/__init__.py"
        DESTINATION "${SITE_PACKAGE}/${BINDING_NAME}${pyside2_SUFFIX}")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/_utils.py
        DESTINATION "${SITE_PACKAGE}/${BINDING_NAME}${pyside2_SUFFIX}")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/typesystem_templates.xml
        DESTINATION share/PySide2${pyside_SUFFIX}/typesystems)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pyside2_global.h
        DESTINATION include/${BINDING_NAME}${pyside2_SUFFIX})

