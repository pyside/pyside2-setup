project(QtConcurrent)

set(QtConcurrent_SRC
${QtConcurrent_GEN_DIR}/qtconcurrent_wrapper.cpp
# module is always needed
${QtConcurrent_GEN_DIR}/qtconcurrent_module_wrapper.cpp
)

make_path(QtConcurrent_typesystem_path  ${QtCore_SOURCE_DIR}
                                        ${QtCore_BINARY_DIR}
                                        ${QtConcurrent_SOURCE_DIR})

set(QtConcurrent_include_dirs   ${QtConcurrent_SOURCE_DIR}
                                ${QtConcurrent_BINARY_DIR}
                                ${Qt5Core_INCLUDE_DIRS}
                                ${Qt5Concurrent_INCLUDE_DIRS}
                                ${SHIBOKEN_INCLUDE_DIR}
                                ${libpyside_SOURCE_DIR}
                                ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                                ${QtCore_GEN_DIR}
                                )
set(QtConcurrent_libraries  pyside2
                            ${SHIBOKEN_PYTHON_LIBRARIES}
                            ${SHIBOKEN_LIBRARY}
                            ${QtConcurrent_LIBRARIES}
                            )
set(QtConcurrent_deps QtCore)

create_pyside_module(QtConcurrent
                     QtConcurrent_include_dirs
                     QtConcurrent_libraries
                     QtConcurrent_deps
                     QtConcurrent_typesystem_path
                     QtConcurrent_SRC
                     "")
