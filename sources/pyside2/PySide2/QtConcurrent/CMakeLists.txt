project(QtConcurrent)

set(QtConcurrent_SRC
${QtConcurrent_GEN_DIR}/qtconcurrent_wrapper.cpp
# module is always needed
${QtConcurrent_GEN_DIR}/qtconcurrent_module_wrapper.cpp
)

set(QtConcurrent_include_dirs   ${QtConcurrent_SOURCE_DIR}
                                ${QtConcurrent_BINARY_DIR}
                                ${Qt5Core_INCLUDE_DIRS}
                                ${Qt5Concurrent_INCLUDE_DIRS}
                                ${libpyside_SOURCE_DIR}
                                ${QtCore_GEN_DIR}
                                )
set(QtConcurrent_libraries  pyside2
                            ${QtConcurrent_LIBRARIES}
                            ${Qt5Core_LIBRARIES}
                            )
set(QtConcurrent_deps QtCore)

create_pyside_module(NAME QtConcurrent
                     INCLUDE_DIRS QtConcurrent_include_dirs
                     LIBRARIES QtConcurrent_libraries
                     DEPS QtConcurrent_deps
                     TYPESYSTEM_PATH QtConcurrent_SOURCE_DIR
                     SOURCES QtConcurrent_SRC)
