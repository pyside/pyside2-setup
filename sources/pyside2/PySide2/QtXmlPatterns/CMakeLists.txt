project(QtXmlPatterns)

set(QtXmlPatterns_SRC
${QtXmlPatterns_GEN_DIR}/qabstractmessagehandler_wrapper.cpp
${QtXmlPatterns_GEN_DIR}/qabstracturiresolver_wrapper.cpp
${QtXmlPatterns_GEN_DIR}/qabstractxmlnodemodel_wrapper.cpp
${QtXmlPatterns_GEN_DIR}/qabstractxmlreceiver_wrapper.cpp
${QtXmlPatterns_GEN_DIR}/qsourcelocation_wrapper.cpp
${QtXmlPatterns_GEN_DIR}/qxmlformatter_wrapper.cpp
${QtXmlPatterns_GEN_DIR}/qxmlitem_wrapper.cpp
${QtXmlPatterns_GEN_DIR}/qxmlnamepool_wrapper.cpp
${QtXmlPatterns_GEN_DIR}/qxmlname_wrapper.cpp
${QtXmlPatterns_GEN_DIR}/qxmlnodemodelindex_wrapper.cpp
${QtXmlPatterns_GEN_DIR}/qxmlquery_wrapper.cpp
${QtXmlPatterns_GEN_DIR}/qxmlresultitems_wrapper.cpp
${QtXmlPatterns_GEN_DIR}/qxmlschemavalidator_wrapper.cpp
${QtXmlPatterns_GEN_DIR}/qxmlschema_wrapper.cpp
${QtXmlPatterns_GEN_DIR}/qxmlserializer_wrapper.cpp
## always needed
${QtXmlPatterns_GEN_DIR}/qtxmlpatterns_module_wrapper.cpp
)

set(QtXmlPatterns_include_dirs  ${QtXmlPatterns_SOURCE_DIR}
                                ${QtXmlPatterns_BINARY_DIR}
                                ${Qt5Core_INCLUDE_DIRS}
                                ${Qt5XmlPatterns_INCLUDE_DIRS}
                                ${libpyside_SOURCE_DIR}
                                ${QtCore_GEN_DIR}
                                )
set(QtXmlPatterns_libraries     pyside2
                                ${Qt5Core_LIBRARIES}
                                ${Qt5XmlPatterns_LIBRARIES})
set(QtXmlPatterns_deps QtCore)

create_pyside_module(NAME QtXmlPatterns
                     INCLUDE_DIRS QtXmlPatterns_include_dirs
                     LIBRARIES QtXmlPatterns_libraries
                     DEPS QtXmlPatterns_deps
                     TYPESYSTEM_PATH QtXmlPatterns_SOURCE_DIR
                     SOURCES QtXmlPatterns_SRC)

