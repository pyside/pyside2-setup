project(QtTextToSpeech)

set(QtTextToSpeech_SRC
${QtTextToSpeech_GEN_DIR}/qtexttospeech_wrapper.cpp
${QtTextToSpeech_GEN_DIR}/qtexttospeechengine_wrapper.cpp
${QtTextToSpeech_GEN_DIR}/qvoice_wrapper.cpp
# module is always needed
${QtTextToSpeech_GEN_DIR}/qttexttospeech_module_wrapper.cpp
)

set(QtTextToSpeech_include_dirs ${QtTextToSpeech_SOURCE_DIR}
                                ${QtTextToSpeech_BINARY_DIR}
                                ${Qt5Core_INCLUDE_DIRS}
                                ${Qt5TextToSpeech_INCLUDE_DIRS}
                                ${SHIBOKEN_INCLUDE_DIR}
                                ${libpyside_SOURCE_DIR}
                                ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                                ${QtCore_GEN_DIR})

set(QtTextToSpeech_libraries    pyside2
                                ${SHIBOKEN_PYTHON_LIBRARIES}
                                ${SHIBOKEN_LIBRARY}
                                ${Qt5Multimedia_LIBRARIES}
                                ${Qt5TextToSpeech_LIBRARIES}
                                ${Qt5Core_LIBRARIES})

set(QtTextToSpeech_deps QtCore QtMultimedia)

create_pyside_module(NAME QtTextToSpeech
                     INCLUDE_DIRS QtTextToSpeech_include_dirs
                     LIBRARIES QtTextToSpeech_libraries
                     DEPS QtTextToSpeech_deps
                     TYPESYSTEM_PATH QtTextToSpeech_SOURCE_DIR
                     SOURCES QtTextToSpeech_SRC)
