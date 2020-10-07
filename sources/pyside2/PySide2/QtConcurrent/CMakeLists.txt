project(QtConcurrent)

set(QtConcurrent_SRC
${QtConcurrent_GEN_DIR}/qtconcurrent_wrapper.cpp
${QtConcurrent_GEN_DIR}/qfuturevoid_wrapper.cpp
${QtConcurrent_GEN_DIR}/qfutureqstring_wrapper.cpp
${QtConcurrent_GEN_DIR}/qfuturewatchervoid_wrapper.cpp
${QtConcurrent_GEN_DIR}/qfuturewatcherqstring_wrapper.cpp
# module is always needed
${QtConcurrent_GEN_DIR}/qtconcurrent_module_wrapper.cpp
)

set(QtConcurrent_include_dirs   ${QtConcurrent_SOURCE_DIR}
                                ${QtConcurrent_BINARY_DIR}
                                ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                                ${Qt${QT_MAJOR_VERSION}Concurrent_INCLUDE_DIRS}
                                ${libpyside_SOURCE_DIR}
                                ${QtCore_GEN_DIR}
                                )
set(QtConcurrent_libraries  pyside2
                            ${QtConcurrent_LIBRARIES}
                            ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                            )
set(QtConcurrent_deps QtCore)

create_pyside_module(NAME QtConcurrent
                     INCLUDE_DIRS QtConcurrent_include_dirs
                     LIBRARIES QtConcurrent_libraries
                     DEPS QtConcurrent_deps
                     TYPESYSTEM_PATH QtConcurrent_SOURCE_DIR
                     SOURCES QtConcurrent_SRC)
