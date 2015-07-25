project(QtXml)

set(QtXml_SRC
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qdomattr_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qdomcdatasection_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qdomcharacterdata_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qdomcomment_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qdomdocument_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qdomdocumentfragment_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qdomdocumenttype_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qdomelement_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qdomentity_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qdomentityreference_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qdomimplementation_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qdomnamednodemap_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qdomnode_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qdomnodelist_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qdomnotation_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qdomprocessinginstruction_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qdomtext_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qxmlattributes_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qxmlcontenthandler_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qxmldeclhandler_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qxmldefaulthandler_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qxmldtdhandler_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qxmlentityresolver_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qxmlerrorhandler_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qxmlinputsource_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qxmllexicalhandler_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qxmllocator_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qxmlnamespacesupport_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qxmlparseexception_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qxmlreader_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qxmlsimplereader_wrapper.cpp
# module is always needed
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXml/qtxml_module_wrapper.cpp
)

make_path(QtXml_typesystem_path ${QtCore_SOURCE_DIR} ${QtCore_BINARY_DIR}
                                ${QtXml_SOURCE_DIR})

set(QtXml_include_dirs      ${QtXml_SOURCE_DIR}
                            ${QtXml_BINARY_DIR}
                            ${CMAKE_SOURCE_DIR}
                            ${Qt5Core_INCLUDE_DIRS}
                            ${Qt5Xml_INCLUDE_DIRS}
                            ${SHIBOKEN_INCLUDE_DIR}
                            ${libpyside_SOURCE_DIR}
                            ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                            ${QtCore_BINARY_DIR}/PySide/QtCore
                            )
set(QtXml_libraries         pyside
                            ${SHIBOKEN_PYTHON_LIBRARIES}
                            ${SHIBOKEN_LIBRARY}
                            ${Qt5Xml_LIBRARIES})
set(QtXml_deps QtCore)

create_pyside_module(QtXml
                     QtXml_include_dirs
                     QtXml_libraries
                     QtXml_deps
                     QtXml_typesystem_path
                     QtXml_SRC
                     "")
