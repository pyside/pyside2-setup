project(sample)

set(sample_TYPESYSTEM
${CMAKE_CURRENT_SOURCE_DIR}/typesystem_sample.xml
)

set(sample_SRC
${CMAKE_CURRENT_BINARY_DIR}/sample/abstractmodifications_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/abstract_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/derived_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/implicitconv_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/listuser_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/modifications_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/mapuser_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/pairuser_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/point_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/reference_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/sample_module_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/samplenamespace_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/size_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/virtualmethods_wrapper.cpp
)

add_custom_command(OUTPUT ${sample_SRC}
COMMAND ${CMAKE_BINARY_DIR}/shiboken
        ${CMAKE_CURRENT_SOURCE_DIR}/global.h
        --include-paths=${libsample_SOURCE_DIR}
        --typesystem-paths=${CMAKE_CURRENT_SOURCE_DIR}
        --output-directory=${CMAKE_CURRENT_BINARY_DIR}
        ${sample_TYPESYSTEM}
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
COMMENT "Running generator for test binding..."
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}
                    ${PYTHON_INCLUDE_PATH}
                    ${libsample_SOURCE_DIR}
                    ${libshiboken_SOURCE_DIR})
add_library(sample MODULE ${sample_SRC})
set_property(TARGET sample PROPERTY PREFIX "")
target_link_libraries(sample
                      libsample
                      ${PYTHON_LIBRARIES}
                      libshiboken)

add_dependencies(sample shiboken)

