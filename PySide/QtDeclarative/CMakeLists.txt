project(QtDeclarative)

set(QtDeclarative_registerType "${CMAKE_CURRENT_SOURCE_DIR}/pysideqmlregistertype.cpp")

set(QtDeclarative_SRC
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtDeclarative/qdeclarativecomponent_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtDeclarative/qdeclarativecontext_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtDeclarative/qdeclarativeengine_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtDeclarative/qdeclarativeerror_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtDeclarative/qdeclarativeexpression_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtDeclarative/qdeclarativeextensioninterface_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtDeclarative/qdeclarativeextensionplugin_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtDeclarative/qdeclarativeimageprovider_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtDeclarative/qdeclarativeitem_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtDeclarative/qdeclarativelistreference_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtDeclarative/qdeclarativenetworkaccessmanagerfactory_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtDeclarative/qdeclarativeparserstatus_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtDeclarative/qdeclarativepropertymap_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtDeclarative/qdeclarativepropertyvaluesource_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtDeclarative/qdeclarativeproperty_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtDeclarative/qdeclarativescriptstring_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtDeclarative/qdeclarativeview_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/PySide/QtDeclarative/qtdeclarative_module_wrapper.cpp
)

make_path(QtDeclarative_typesystem_path ${QtCore_SOURCE_DIR} ${QtGui_SOURCE_DIR} ${QtNetwork_SOURCE_DIR}
                                        ${QtCore_BINARY_DIR} ${QtGui_BINARY_DIR} ${QtNetwork_BINARY_DIR}
                                        ${QtDeclarative_SOURCE_DIR})

set(QtDeclarative_include_dirs  ${CMAKE_CURRENT_SOURCE_DIR}
                            ${Qt5Core_INCLUDE_DIRS}
                            ${Qt5Gui_INCLUDE_DIRS}
                            ${Qt5Network_INCLUDE_DIRS}
                            ${Qt5Declarative_INCLUDE_DIRS}
                            ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                            ${SHIBOKEN_INCLUDE_DIR}
                            ${libpyside_SOURCE_DIR}
                            ${QtGui_BINARY_DIR}/PySide/QtGui/
                            ${QtCore_BINARY_DIR}/PySide/QtCore/
                            ${QtNetwork_BINARY_DIR}/PySide/QtNetwork/
                            ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtDeclarative)

set(QtDeclarative_libraries   pyside
                              ${SHIBOKEN_PYTHON_LIBRARIES}
                              ${Qt5Core_LIBRARIES}
                              ${Qt5Gui_LIBRARIES}
                              ${Qt5Network_LIBRARIES}
                              ${Qt5Script_LIBRARIES}
                              ${Qt5Svg_LIBRARIES}
                              ${Qt5Sql_LIBRARIES}
                              ${Qt5XmlPatterns_LIBRARIES}
                              ### deprecated ${QT_QTOPENGL_LIBRARIES}
                              ${Qt5DeclarativeE_LIBRARIES})

set(QtDeclarative_deps QtGui QtNetwork)

create_pyside_module(QtDeclarative
                     QtDeclarative_include_dirs
                     QtDeclarative_libraries
                     QtDeclarative_deps
                     QtDeclarative_typesystem_path
                     QtDeclarative_SRC
                     QtDeclarative_registerType)

