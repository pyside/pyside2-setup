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

make_path(QtXmlPatterns_typesystem_path ${QtCore_SOURCE_DIR} ${QtCore_BINARY_DIR}
                                        ${QtXmlPatterns_SOURCE_DIR})

set(QtXmlPatterns_include_dirs  ${QtXmlPatterns_SOURCE_DIR}
                                ${QtXmlPatterns_BINARY_DIR}
                                ${Qt5Core_INCLUDE_DIRS}
                                ${Qt5XmlPatterns_INCLUDE_DIRS}
                                ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                                ${SHIBOKEN_INCLUDE_DIR}
                                ${libpyside_SOURCE_DIR}
                                ${QtCore_GEN_DIR}
                                )
set(QtXmlPatterns_libraries     pyside2
                                ${SHIBOKEN_PYTHON_LIBRARIES}
                                ${Qt5Core_LIBRARIES}
                                ${Qt5XmlPatterns_LIBRARIES})
set(QtXmlPatterns_deps QtCore)

create_pyside_module(QtXmlPatterns
                     QtXmlPatterns_include_dirs
                     QtXmlPatterns_libraries
                     QtXmlPatterns_deps
                     QtXmlPatterns_typesystem_path
                     QtXmlPatterns_SRC
                     "")

