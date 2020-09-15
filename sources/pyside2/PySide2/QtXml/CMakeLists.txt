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
# module is always needed
${QtXml_GEN_DIR}/qtxml_module_wrapper.cpp
)

set(QtXml_include_dirs      ${QtXml_SOURCE_DIR}
                            ${QtXml_BINARY_DIR}
                            ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                            ${Qt${QT_MAJOR_VERSION}Xml_INCLUDE_DIRS}
                            ${libpyside_SOURCE_DIR}
                            ${QtCore_GEN_DIR}
                            )
set(QtXml_libraries         pyside2
                            ${Qt${QT_MAJOR_VERSION}Xml_LIBRARIES})
set(QtXml_deps QtCore)

create_pyside_module(NAME QtXml
                     INCLUDE_DIRS QtXml_include_dirs
                     LIBRARIES QtXml_libraries
                     DEPS QtXml_deps
                     TYPESYSTEM_PATH QtXml_SOURCE_DIR
                     SOURCES QtXml_SRC)
