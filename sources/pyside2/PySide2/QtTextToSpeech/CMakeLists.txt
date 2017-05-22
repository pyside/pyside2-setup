project(QtTextToSpeech)

set(QtTextToSpeech_SRC
${QtTextToSpeech_GEN_DIR}/qtexttospeech_wrapper.cpp
${QtTextToSpeech_GEN_DIR}/qtexttospeechengine_wrapper.cpp
${QtTextToSpeech_GEN_DIR}/qvoice_wrapper.cpp
# module is always needed
${QtTextToSpeech_GEN_DIR}/qttexttospeech_module_wrapper.cpp
)

make_path(QtTextToSpeech_typesystem_path
    ${QtCore_SOURCE_DIR}
    ${QtCore_BINARY_DIR}
    ${QtTextToSpeech_SOURCE_DIR})

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

create_pyside_module(QtTextToSpeech
                     QtTextToSpeech_include_dirs
                     QtTextToSpeech_libraries
                     QtTextToSpeech_deps
                     QtTextToSpeech_typesystem_path
                     QtTextToSpeech_SRC
                     "")
