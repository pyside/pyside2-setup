project(QtScript)

set(QtScript_SRC
${QtScript_GEN_DIR}/qscriptable_wrapper.cpp
${QtScript_GEN_DIR}/qscriptclass_wrapper.cpp
${QtScript_GEN_DIR}/qscriptclasspropertyiterator_wrapper.cpp
${QtScript_GEN_DIR}/qscriptcontext_wrapper.cpp
${QtScript_GEN_DIR}/qscriptcontextinfo_wrapper.cpp
${QtScript_GEN_DIR}/qscriptengineagent_wrapper.cpp
${QtScript_GEN_DIR}/qscriptprogram_wrapper.cpp
${QtScript_GEN_DIR}/qscriptengine_wrapper.cpp
${QtScript_GEN_DIR}/qscriptextensioninterface_wrapper.cpp
${QtScript_GEN_DIR}/qscriptextensionplugin_wrapper.cpp
${QtScript_GEN_DIR}/qscriptstring_wrapper.cpp
${QtScript_GEN_DIR}/qscriptvalue_wrapper.cpp
${QtScript_GEN_DIR}/qscriptvalueiterator_wrapper.cpp
# module is always needed
${QtScript_GEN_DIR}/qtscript_module_wrapper.cpp
)

set(QtScript_glue_sources
    "${QtScript_SOURCE_DIR}/qscript_value_iterator_glue.cpp"
)

set(QtScript_include_dirs   ${QtScript_SOURCE_DIR}
                            ${QtScript_BINARY_DIR}
                            ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                            ${Qt${QT_MAJOR_VERSION}Script_INCLUDE_DIRS}
                            ${libpyside_SOURCE_DIR}
                            ${QtCore_GEN_DIR}
                            )
set(QtScript_libraries      pyside2
                            ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                            ${Qt${QT_MAJOR_VERSION}Script_LIBRARIES})
set(QtScript_deps QtCore)
create_pyside_module(NAME QtScript
                     INCLUDE_DIRS QtScript_include_dirs
                     LIBRARIES QtScript_libraries
                     DEPS QtScript_deps
                     TYPESYSTEM_PATH QtScript_SOURCE_DIR
                     SOURCES QtScript_SRC
                     GLUE_SOURCES QtScript_glue_sources)
