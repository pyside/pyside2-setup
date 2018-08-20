cmake_minimum_required(VERSION 3.1)
cmake_policy(VERSION 3.1)

project(doc)

if (WIN32)
    set(PATH_SEP "\;")
else()
    set(PATH_SEP ":")
endif()

set(DOC_DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/qdoc-output")

get_filename_component(ROOT ${CMAKE_CURRENT_BINARY_DIR} DIRECTORY)
set(TS_ROOT "${ROOT}/PySide2")

file(REMOVE ${CMAKE_CURRENT_LIST_DIR}/pyside.qdocconf ${CMAKE_CURRENT_LIST_DIR}/pyside.qdocconf.in)

# For Qt modules that are part of the documentation build:
#    - Configure the module docconf file
#    - Write shiboken header consisting of pyside2_global.h and module includes
#    - Build include path for qdoc for shiboken

# The last element of the include list is the mkspec directory containing qplatformdefs.h
list(GET Qt5Core_INCLUDE_DIRS -1 mkspecInclude)
configure_file("pyside-config.qdocconf.in" "${CMAKE_CURRENT_LIST_DIR}/pyside-config.qdocconf" @ONLY)

file(READ "${pyside2_BINARY_DIR}/pyside2_global.h" docHeaderContents)
file(READ "typesystem_doc.xml.in" typeSystemDocXmlContents)

foreach(moduleIn ${all_module_shortnames})
    string(TOLOWER "${moduleIn}" lowerModuleIn)
    set(docConf "${CMAKE_CURRENT_LIST_DIR}/qtmodules/pyside-qt${lowerModuleIn}.qdocconf.in")
    if(EXISTS "${docConf}")
        string(REGEX REPLACE "(^.*)\.in" "\\1" OUTFILE ${docConf})
        get_filename_component(BASENAME ${OUTFILE} NAME)
        configure_file(${docConf} "${CMAKE_CURRENT_LIST_DIR}/qtmodules/${BASENAME}" @ONLY)
        file(APPEND "pyside.qdocconf.in" "\@CMAKE_CURRENT_LIST_DIR\@/qtmodules/${BASENAME}\n")
        # Handle docconf files in Qt that contain multiple modules
        if ("${moduleIn}" STREQUAL "3DExtras")
            set(modules 3DCore 3DRender 3DInput 3DLogic 3DAnimation "${moduleIn}")
        elseif ("${moduleIn}" STREQUAL "QuickWidgets")
            set(modules Qml Quick "${moduleIn}")
        elseif ("${moduleIn}" STREQUAL "MultimediaWidgets")
            set(modules Multimedia "${moduleIn}")
        elseif ("${moduleIn}" STREQUAL "WebEngineWidgets")
            set(modules WebEngine WebEngineCore "${moduleIn}")
        else()
            set(modules "${moduleIn}")
        endif()
        foreach(module ${modules})
            string(TOLOWER "${module}" lowerModule)
            # -- @TODO fix this for macOS frameworks.
            file(APPEND "${CMAKE_CURRENT_LIST_DIR}/pyside-config.qdocconf"
                 "    -I ${QT_INCLUDE_DIR}Qt${module} \\\n"
                 "    -I ${QT_INCLUDE_DIR}Qt${module}/${Qt5Core_VERSION} \\\n"
                 "    -I ${QT_INCLUDE_DIR}Qt${module}/${Qt5Core_VERSION}/Qt${module} \\\n")

            if (${moduleIn} STREQUAL "X11Extras")
                set(globalHeader "QX11Info")
            else()
                set(globalHeader "Qt${module}")
            endif()
            set(docHeaderContents "${docHeaderContents}\n#include <Qt${module}/${globalHeader}>")
            set(typeSystemDocXmlContents "${typeSystemDocXmlContents}\n<load-typesystem name=\"Qt${module}/typesystem_${lowerModule}.xml\" generate=\"yes\"/>")
        endforeach()
    endif()
endforeach()

set(typeSystemDocXmlContents "${typeSystemDocXmlContents}\n</typesystem>\n")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/typesystem_doc.xml" "${typeSystemDocXmlContents}")

set(docHeader "${pyside2_BINARY_DIR}/qdoc.h")
file(WRITE ${docHeader} "${docHeaderContents}")

configure_file("pyside.qdocconf.in" "pyside.qdocconf" @ONLY)

set(QDOC_TYPESYSTEM_PATH "${pyside2_SOURCE_DIR}${PATH_SEP}${pyside2_BINARY_DIR}")

add_custom_target(qdoc
                # Use dummy Qt version information, QDoc needs it but has no effect on WebXML output
                COMMAND ${CMAKE_COMMAND} -E env BUILDDIR=${CMAKE_CURRENT_LIST_DIR}/src QT_INSTALL_DOCS=${QT_SRC_DIR}/doc QT_VERSION=1.0.0 QT_VER=1.0 QT_VERSION_TAG=100
                qdoc pyside.qdocconf -single-exec -installdir ${DOC_DATA_DIR} -outputdir ${DOC_DATA_DIR}
                COMMENT "Running qdoc against Qt source code..."
                SOURCE "pyside.qdocconf")

add_custom_target(apidoc
                  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/rst
                  COMMAND ${SHIBOKEN_PYTHON_INTERPRETER} ${SPHINX_BUILD} -b html  ${CMAKE_CURRENT_BINARY_DIR}/rst html
                 )

# create conf.py based on conf.py.in
configure_file("conf.py.in" "rst/conf.py" @ONLY)

add_custom_target("docrsts"
    COMMAND ${SHIBOKEN_BINARY} --generator-set=qtdoc ${docHeader}
            --include-paths="${QT_INCLUDE_DIR}${PATH_SEP}${pyside2_SOURCE_DIR}${PATH_SEP}${TS_ROOT}"
            --api-version=${SUPPORTED_QT_VERSION}
            --typesystem-paths="${QDOC_TYPESYSTEM_PATH}"
            --library-source-dir=${QT_SRC_DIR}
            --documentation-data-dir=${DOC_DATA_DIR}/webxml
            --output-directory=${CMAKE_CURRENT_BINARY_DIR}/rst
            --documentation-code-snippets-dir=${CMAKE_CURRENT_SOURCE_DIR}/codesnippets/doc/src/snippets${PATH_SEP}${CMAKE_CURRENT_SOURCE_DIR}/codesnippets/examples
            --documentation-extra-sections-dir=${CMAKE_CURRENT_SOURCE_DIR}/extras
            --additional-documentation=${CMAKE_CURRENT_LIST_DIR}/additionaldocs.lst
            ${CMAKE_CURRENT_BINARY_DIR}/typesystem_doc.xml
    WORKING_DIRECTORY ${${module}_SOURCE_DIR}
    COMMENT "Running generator to generate documentation..."
)

add_custom_target("licensedocrsts"
    COMMAND ${PYTHON_EXECUTABLE}
            ${CMAKE_CURRENT_LIST_DIR}/qtattributionsscannertorst.py
            ${CMAKE_CURRENT_LIST_DIR}/../../..
            ${CMAKE_CURRENT_BINARY_DIR}/rst/licenses.rst
    COMMENT "Creating 3rdparty license documentation..."
)

add_dependencies(apidoc docrsts licensedocrsts)
add_dependencies(licensedocrsts docrsts)
add_dependencies(docrsts qdoc)

# #create devhelp file
# add_custom_target(apidevhelp
#     COMMAND python;${CMAKE_CURRENT_SOURCE_DIR}/pyhtml2devhelp.py;${CMAKE_BINARY_DIR}/apidoc/html;index.html >
#             ${CMAKE_BINARY_DIR}/apidoc/html/PySide.devhelp;${BINDING_API_VERSION}&&;
#             gzip;-9v;-f;${CMAKE_BINARY_DIR}/apidoc/html/PySide.devhelp
#     COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_INSTALL_PREFIX}/share/devhelp/books"
#     COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_INSTALL_PREFIX}/share/doc/${BINDING_NAME}/html" "${CMAKE_INSTALL_PREFIX}/share/devhelp/books/${BINDING_NAME}"
# )

#install files
add_custom_target(apidocinstall
    COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/share/doc/PySide2-${BINDING_API_VERSION} && cp -rv ${CMAKE_CURRENT_BINARY_DIR}/html/* ${CMAKE_INSTALL_PREFIX}/share/doc/PySide-${BINDING_API_VERSION}
)

add_dependencies(apidocinstall apidoc)
# add_dependencies(apidocinstall apidevhelp)

