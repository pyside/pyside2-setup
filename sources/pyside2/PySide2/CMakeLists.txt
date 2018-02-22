project(pyside2)

# Configure include based on platform
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/global.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/pyside2_global.h" @ONLY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
               "${CMAKE_CURRENT_BINARY_DIR}/__init__.py" @ONLY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/_config.py.in"
              "${CMAKE_CURRENT_BINARY_DIR}/_config.py" @ONLY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/__init__.py"
               "${CMAKE_CURRENT_BINARY_DIR}/support/__init__.py" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/signature/__init__.py"
               "${CMAKE_CURRENT_BINARY_DIR}/support/signature/__init__.py" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/signature/loader.py"
               "${CMAKE_CURRENT_BINARY_DIR}/support/signature/loader.py" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/signature/mapping.py"
               "${CMAKE_CURRENT_BINARY_DIR}/support/signature/mapping.py" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/signature/parser.py"
               "${CMAKE_CURRENT_BINARY_DIR}/support/signature/parser.py" COPYONLY)
if (PYTHON_VERSION_MAJOR EQUAL 3)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/signature/inspect.py"
                   "${CMAKE_CURRENT_BINARY_DIR}/support/signature/inspect.py" COPYONLY)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/signature/typing36.py"
                   "${CMAKE_CURRENT_BINARY_DIR}/support/signature/typing.py" COPYONLY)
else()
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/signature/backport_inspect.py"
                   "${CMAKE_CURRENT_BINARY_DIR}/support/signature/backport_inspect.py" COPYONLY)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/signature/typing27.py"
                   "${CMAKE_CURRENT_BINARY_DIR}/support/signature/typing.py" COPYONLY)
endif()

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
        DESTINATION "${PYTHON_SITE_PACKAGES}/${BINDING_NAME}${pyside2_SUFFIX}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/_config.py"
        DESTINATION "${PYTHON_SITE_PACKAGES}/${BINDING_NAME}${pyside2_SUFFIX}")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/typesystem_templates.xml
        DESTINATION share/PySide2${pyside_SUFFIX}/typesystems)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pyside2_global.h
        DESTINATION include/${BINDING_NAME}${pyside2_SUFFIX})

