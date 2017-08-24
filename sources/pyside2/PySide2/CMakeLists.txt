project(pyside2)

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
        DESTINATION "${PYTHON_SITE_PACKAGES}/${BINDING_NAME}${pyside2_SUFFIX}")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/_utils.py
        DESTINATION "${PYTHON_SITE_PACKAGES}/${BINDING_NAME}${pyside2_SUFFIX}")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/typesystem_templates.xml
        DESTINATION share/PySide2${pyside_SUFFIX}/typesystems)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pyside2_global.h
        DESTINATION include/${BINDING_NAME}${pyside2_SUFFIX})

