project(QtXml)

set(QtXml_SRC
${QtXml_GEN_DIR}/qdomattr_wrapper.cpp
${QtXml_GEN_DIR}/qdomcdatasection_wrapper.cpp
${QtXml_GEN_DIR}/qdomcharacterdata_wrapper.cpp
${QtXml_GEN_DIR}/qdomcomment_wrapper.cpp
${QtXml_GEN_DIR}/qdomdocument_wrapper.cpp
${QtXml_GEN_DIR}/qdomdocumentfragment_wrapper.cpp
${QtXml_GEN_DIR}/qdomdocumenttype_wrapper.cpp
${QtXml_GEN_DIR}/qdomelement_wrapper.cpp
${QtXml_GEN_DIR}/qdomentity_wrapper.cpp
${QtXml_GEN_DIR}/qdomentityreference_wrapper.cpp
${QtXml_GEN_DIR}/qdomimplementation_wrapper.cpp
${QtXml_GEN_DIR}/qdomnamednodemap_wrapper.cpp
${QtXml_GEN_DIR}/qdomnode_wrapper.cpp
${QtXml_GEN_DIR}/qdomnodelist_wrapper.cpp
${QtXml_GEN_DIR}/qdomnotation_wrapper.cpp
${QtXml_GEN_DIR}/qdomprocessinginstruction_wrapper.cpp
${QtXml_GEN_DIR}/qdomtext_wrapper.cpp
${QtXml_GEN_DIR}/qxmlattributes_wrapper.cpp
${QtXml_GEN_DIR}/qxmlcontenthandler_wrapper.cpp
${QtXml_GEN_DIR}/qxmldeclhandler_wrapper.cpp
${QtXml_GEN_DIR}/qxmldefaulthandler_wrapper.cpp
${QtXml_GEN_DIR}/qxmldtdhandler_wrapper.cpp
${QtXml_GEN_DIR}/qxmlentityresolver_wrapper.cpp
${QtXml_GEN_DIR}/qxmlerrorhandler_wrapper.cpp
${QtXml_GEN_DIR}/qxmlinputsource_wrapper.cpp
${QtXml_GEN_DIR}/qxmllexicalhandler_wrapper.cpp
${QtXml_GEN_DIR}/qxmllocator_wrapper.cpp
${QtXml_GEN_DIR}/qxmlnamespacesupport_wrapper.cpp
${QtXml_GEN_DIR}/qxmlparseexception_wrapper.cpp
${QtXml_GEN_DIR}/qxmlreader_wrapper.cpp
${QtXml_GEN_DIR}/qxmlsimplereader_wrapper.cpp
# module is always needed
${QtXml_GEN_DIR}/qtxml_module_wrapper.cpp
)

set(QtXml_include_dirs      ${QtXml_SOURCE_DIR}
                            ${QtXml_BINARY_DIR}
                            ${CMAKE_SOURCE_DIR}
                            ${Qt5Core_INCLUDE_DIRS}
                            ${Qt5Xml_INCLUDE_DIRS}
                            ${SHIBOKEN_INCLUDE_DIR}
                            ${libpyside_SOURCE_DIR}
                            ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                            ${QtCore_GEN_DIR}
                            )
set(QtXml_libraries         pyside2
                            ${SHIBOKEN_PYTHON_LIBRARIES}
                            ${SHIBOKEN_LIBRARY}
                            ${Qt5Xml_LIBRARIES})
set(QtXml_deps QtCore)

create_pyside_module(NAME QtXml
                     INCLUDE_DIRS QtXml_include_dirs
                     LIBRARIES QtXml_libraries
                     DEPS QtXml_deps
                     TYPESYSTEM_PATH QtXml_SOURCE_DIR
                     SOURCES QtXml_SRC)
