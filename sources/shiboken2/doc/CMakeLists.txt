cmake_minimum_required(VERSION 3.16)

find_program(SPHINX sphinx-build DOC "Path to sphinx-build binary.")

if (SPHINX)
    message(STATUS "sphinx-build - found")
    configure_file(conf.py.in conf.py @ONLY)
    # conditional tag for sphinx build
    #string(JOIN "_" SPHINX_TAG ${DOC_OUTPUT_FORMAT} "format")
    add_custom_target(doc
        COMMAND ${SPHINX} -b ${DOC_OUTPUT_FORMAT} -c . ${CMAKE_CURRENT_SOURCE_DIR} html
        COMMENT "Generating HTMLs..."
        VERBATIM)
    # Python script that will be called to update the QHP
    set(py_cmd "from  __future__ import print_function
import fileinput
import re
try:
\tfor line in fileinput.input('html/Shiboken.qhp',inplace=True,backup='.bak'):
\t\tline_copy=line.strip()
\t\tif not line_copy: # check for empty line
\t\t\tcontinue
\t\tmatch=re.match('(^.*virtualFolder.)doc(.*$)',line)
\t\tif match:
\t\t\trepl=''.join([match.group(1),'shiboken2',match.group(2)])
\t\t\tprint(line.replace(match.group(0),repl),end=' ')
\t\telse:
\t\t\tprint(line)
except:
\tpass\n")
    file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/py_script.py
        CONTENT ${py_cmd})

    # create a custom command to generate QCH
    if(DOC_OUTPUT_FORMAT STREQUAL "qthelp")
        file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/html/Shiboken.qhp QHP_FILE)
        add_custom_command(TARGET doc POST_BUILD
            COMMAND ${PYTHON_EXECUTABLE} py_script.py # ${CMAKE_CURRENT_BINARY_DIR}/html/Shiboken.qhp
            COMMAND qhelpgenerator ${QHP_FILE}
            COMMENT "Genereting QCH based on the QHP..."
            VERBATIM)
    endif()
else()
    message(WARNING "sphinx-build - not found! doc target disabled")
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
