project(QtXmlPatterns)

set(QtXmlPatterns_SRC
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXmlPatterns/qabstractmessagehandler_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXmlPatterns/qabstracturiresolver_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXmlPatterns/qabstractxmlnodemodel_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXmlPatterns/qabstractxmlreceiver_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXmlPatterns/qsourcelocation_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXmlPatterns/qxmlformatter_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXmlPatterns/qxmlitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXmlPatterns/qxmlnamepool_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXmlPatterns/qxmlname_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXmlPatterns/qxmlnodemodelindex_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXmlPatterns/qxmlquery_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXmlPatterns/qxmlresultitems_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXmlPatterns/qxmlschemavalidator_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXmlPatterns/qxmlschema_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXmlPatterns/qxmlserializer_wrapper.cpp
## always needed
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtXmlPatterns/qtxmlpatterns_module_wrapper.cpp
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
                                ${QtCore_BINARY_DIR}/PySide/QtCore/
                                )
set(QtXmlPatterns_libraries     pyside
                                ${SHIBOKEN_PYTHON_LIBRARIES}
                                ${Qt5Core_LIBRARIES}
                                ${Qt5XmlPatterns_LIBRARY})
set(QtXmlPatterns_deps QtCore)

create_pyside_module(QtXmlPatterns
                     QtXmlPatterns_include_dirs
                     QtXmlPatterns_libraries
                     QtXmlPatterns_deps
                     QtXmlPatterns_typesystem_path
                     QtXmlPatterns_SRC
                     "")

