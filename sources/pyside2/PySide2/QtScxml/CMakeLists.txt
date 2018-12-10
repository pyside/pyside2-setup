project(QtScxml)

set(QtScxml_OPTIONAL_SRC )
set(QtScxml_DROPPED_ENTRIES )

set(QtScxml_SRC
${QtScxml_GEN_DIR}/qscxmlcompiler_wrapper.cpp
${QtScxml_GEN_DIR}/qscxmlcompiler_loader_wrapper.cpp
${QtScxml_GEN_DIR}/qscxmlevent_wrapper.cpp
${QtScxml_GEN_DIR}/qscxmldynamicscxmlservicefactory_wrapper.cpp
${QtScxml_GEN_DIR}/qscxmlinvokableservice_wrapper.cpp
${QtScxml_GEN_DIR}/qscxmlinvokableservicefactory_wrapper.cpp
${QtScxml_GEN_DIR}/qscxmlstaticscxmlservicefactory_wrapper.cpp
${QtScxml_GEN_DIR}/qscxmlstatemachine_wrapper.cpp
${QtScxml_GEN_DIR}/qscxmltabledata_wrapper.cpp
${QtScxml_GEN_DIR}/qscxmlerror_wrapper.cpp
${QtScxml_GEN_DIR}/qscxmlexecutablecontent_wrapper.cpp
${QtScxml_GEN_DIR}/qscxmlexecutablecontent_assignmentinfo_wrapper.cpp
${QtScxml_GEN_DIR}/qscxmlexecutablecontent_evaluatorinfo_wrapper.cpp
${QtScxml_GEN_DIR}/qscxmlexecutablecontent_foreachinfo_wrapper.cpp
${QtScxml_GEN_DIR}/qscxmlexecutablecontent_invokeinfo_wrapper.cpp
${QtScxml_GEN_DIR}/qscxmlexecutablecontent_parameterinfo_wrapper.cpp
# module is always needed
${QtScxml_GEN_DIR}/qtscxml_module_wrapper.cpp
)

if (Qt5Scxml_VERSION VERSION_EQUAL 5.12.0 OR Qt5Scxml_VERSION VERSION_GREATER 5.12.0)
    list(APPEND QtScxml_SRC
         ${QtScxml_GEN_DIR}/qscxmldatamodel_wrapper.cpp
         ${QtScxml_GEN_DIR}/qscxmldatamodel_foreachloopbody_wrapper.cpp
         ${QtScxml_GEN_DIR}/qscxmlecmascriptdatamodel_wrapper.cpp
         ${QtScxml_GEN_DIR}/qscxmlcppdatamodel_wrapper.cpp
         ${QtScxml_GEN_DIR}/qscxmlnulldatamodel_wrapper.cpp)
endif()

set(QtScxml_include_dirs ${QtScxml_SOURCE_DIR}
                         ${QtScxml_BINARY_DIR}
                         ${Qt5Core_INCLUDE_DIRS}
                         ${Qt5Scxml_INCLUDE_DIRS}
                         ${SHIBOKEN_INCLUDE_DIR}
                         ${libpyside_SOURCE_DIR}
                         ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                         ${QtCore_GEN_DIR})

set(QtScxml_libraries pyside2
                         ${SHIBOKEN_PYTHON_LIBRARIES}
                         ${SHIBOKEN_LIBRARY}
                         ${Qt5Scxml_LIBRARIES})

set(QtScxml_deps QtCore)

create_pyside_module(NAME QtScxml
                     INCLUDE_DIRS QtScxml_include_dirs
                     LIBRARIES QtScxml_libraries
                     DEPS QtScxml_deps
                     TYPESYSTEM_PATH QtScxml_SOURCE_DIR
                     SOURCES QtScxml_SRC
                     DROPPED_ENTRIES QtScxml_DROPPED_ENTRIES)
