project(other)

set(other_TYPESYSTEM
${CMAKE_CURRENT_SOURCE_DIR}/typesystem_other.xml
)

set(other_SRC
${CMAKE_CURRENT_BINARY_DIR}/other/extendsnoimplicitconversion_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/other/number_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/other/otherderived_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/other/othermultiplederived_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/other/otherobjecttype_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/other/other_module_wrapper.cpp
)

add_custom_command(OUTPUT ${other_SRC}
COMMAND ${GENERATORRUNNER_BINARY} --generator-set=${generators_BINARY_DIR}/shiboken_generator${CMAKE_RELEASE_POSTFIX}${CMAKE_DEBUG_POSTFIX}
        --enable-parent-ctor-heuristic
        ${CMAKE_CURRENT_SOURCE_DIR}/global.h
        --include-paths=${libother_SOURCE_DIR}${PATH_SEP}${libsample_SOURCE_DIR}${PATH_SEP}${libsample_SOURCE_DIR}/..
        --typesystem-paths=${CMAKE_CURRENT_SOURCE_DIR}${PATH_SEP}${sample_SOURCE_DIR}
        --output-directory=${CMAKE_CURRENT_BINARY_DIR}
        ${other_TYPESYSTEM}
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
COMMENT "Running generator for test binding..."
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}
                    ${SBK_PYTHON_INCLUDE_DIR}
                    ${libother_SOURCE_DIR}
                    ${libsample_SOURCE_DIR}
                    ${libsample_SOURCE_DIR}/..
                    ${sample_BINARY_DIR}
                    ${sample_BINARY_DIR}/sample
                    ${libshiboken_SOURCE_DIR})
add_library(other MODULE ${other_SRC})
set_property(TARGET other PROPERTY PREFIX "")
if(WIN32)
    set_property(TARGET other PROPERTY SUFFIX ".pyd")
endif()
target_link_libraries(other
                      libother
                      libsample
                      ${SBK_PYTHON_LIBRARIES}
                      libshiboken)

add_dependencies(other sample shiboken_generator)

