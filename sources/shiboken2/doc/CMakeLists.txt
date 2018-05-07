find_program(SPHINX sphinx-build DOC "Path to sphinx-build binary.")

if (SPHINX)
    message("-- sphinx-build - found")
    configure_file(conf.py.in conf.py @ONLY)
    add_custom_target(doc ${SPHINX} -b html -c . ${CMAKE_CURRENT_SOURCE_DIR} html )
    add_dependencies(doc doc_apiextractor)
else()
    message("-- sphinx-build - not found! doc target disabled")
    if (WIN32)
        # if jom is used and we have no sphinx, then jom will crash.
        # so for windows, we always create a doc target (until jom gets fixed...)
        add_custom_target(doc echo.
            COMMAND echo +++ This is a fake build, to make 'jom' happy.
            COMMAND echo +++ The documentation was _not_ built!
            COMMAND echo.
        )
    endif()
endif()

if (NOT WIN32)
    file(GLOB manpages "${CMAKE_CURRENT_SOURCE_DIR}/*.1")
    install(FILES ${manpages} DESTINATION share/man/man1)
endif()
