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
                                ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                                ${Qt${QT_MAJOR_VERSION}TextToSpeech_INCLUDE_DIRS}
                                ${libpyside_SOURCE_DIR}
                                ${QtCore_GEN_DIR})

set(QtTextToSpeech_libraries    pyside6
                                ${Qt${QT_MAJOR_VERSION}Multimedia_LIBRARIES}
                                ${Qt${QT_MAJOR_VERSION}TextToSpeech_LIBRARIES}
                                ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES})

set(QtTextToSpeech_deps QtCore QtMultimedia)

create_pyside_module(NAME QtTextToSpeech
                     INCLUDE_DIRS QtTextToSpeech_include_dirs
                     LIBRARIES QtTextToSpeech_libraries
                     DEPS QtTextToSpeech_deps
                     TYPESYSTEM_PATH QtTextToSpeech_SOURCE_DIR
                     SOURCES QtTextToSpeech_SRC)
