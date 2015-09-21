project(QtDeclarative)

set(QtDeclarative_registerType "${QtDeclarative_SOURCE_DIR}/pysideqmlregistertype.cpp")

set(QtDeclarative_SRC
${QtDeclarative_GEN_DIR}/qdeclarativecomponent_wrapper.cpp
${QtDeclarative_GEN_DIR}/qdeclarativecontext_wrapper.cpp
${QtDeclarative_GEN_DIR}/qdeclarativeengine_wrapper.cpp
${QtDeclarative_GEN_DIR}/qdeclarativeerror_wrapper.cpp
${QtDeclarative_GEN_DIR}/qdeclarativeexpression_wrapper.cpp
${QtDeclarative_GEN_DIR}/qdeclarativeextensioninterface_wrapper.cpp
${QtDeclarative_GEN_DIR}/qdeclarativeextensionplugin_wrapper.cpp
${QtDeclarative_GEN_DIR}/qdeclarativeimageprovider_wrapper.cpp
${QtDeclarative_GEN_DIR}/qdeclarativeitem_wrapper.cpp
${QtDeclarative_GEN_DIR}/qdeclarativelistreference_wrapper.cpp
${QtDeclarative_GEN_DIR}/qdeclarativenetworkaccessmanagerfactory_wrapper.cpp
${QtDeclarative_GEN_DIR}/qdeclarativeparserstatus_wrapper.cpp
${QtDeclarative_GEN_DIR}/qdeclarativepropertymap_wrapper.cpp
${QtDeclarative_GEN_DIR}/qdeclarativepropertyvaluesource_wrapper.cpp
${QtDeclarative_GEN_DIR}/qdeclarativeproperty_wrapper.cpp
${QtDeclarative_GEN_DIR}/qdeclarativescriptstring_wrapper.cpp
${QtDeclarative_GEN_DIR}/qdeclarativeview_wrapper.cpp
# module is always needed
${QtDeclarative_GEN_DIR}/qtdeclarative_module_wrapper.cpp
)

make_path(QtDeclarative_typesystem_path ${QtCore_SOURCE_DIR} ${QtGui_SOURCE_DIR} ${QtWidgets_SOURCE_DIR} ${QtNetwork_SOURCE_DIR}
                                        ${QtCore_BINARY_DIR} ${QtGui_BINARY_DIR} ${QtWidgets_BINARY_DIR} ${QtNetwork_BINARY_DIR}
                                        ${QtDeclarative_SOURCE_DIR})

set(QtDeclarative_include_dirs  ${QtDeclarative_SOURCE_DIR}
                                ${Qt5Core_INCLUDE_DIRS}
                                ${Qt5Gui_INCLUDE_DIRS}
                                ${Qt5Network_INCLUDE_DIRS}
                                ${Qt5Declarative_INCLUDE_DIRS}
                                ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                                ${SHIBOKEN_INCLUDE_DIR}
                                ${libpyside_SOURCE_DIR}
                                ${QtWidgets_GEN_DIR}
                                ${QtGui_GEN_DIR}
                                ${QtCore_GEN_DIR}
                                ${QtNetwork_GEN_DIR}
                                ${QtDeclarative_GEN_DIR})

set(QtDeclarative_libraries   pyside2
                              ${SHIBOKEN_PYTHON_LIBRARIES}
                              ${Qt5Core_LIBRARIES}
                              ${Qt5Gui_LIBRARIES}
                              ${Qt5Widgets_LIBRARIES}
                              ${Qt5Network_LIBRARIES}
                              ${Qt5Script_LIBRARIES}
                              ${Qt5Svg_LIBRARIES}
                              ${Qt5Sql_LIBRARIES}
                              ${Qt5XmlPatterns_LIBRARIES}
                              ${Qt5Declarative_LIBRARIES})

set(QtDeclarative_deps QtWidgets QtNetwork)

create_pyside_module(QtDeclarative
                     QtDeclarative_include_dirs
                     QtDeclarative_libraries
                     QtDeclarative_deps
                     QtDeclarative_typesystem_path
                     QtDeclarative_SRC
                     QtDeclarative_registerType)

