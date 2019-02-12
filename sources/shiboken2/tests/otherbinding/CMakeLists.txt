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


configure_file("${CMAKE_CURRENT_SOURCE_DIR}/other-binding.txt.in"
               "${CMAKE_CURRENT_BINARY_DIR}/other-binding.txt" @ONLY)

add_custom_command(
OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/mjb_rejected_classes.log"
BYPRODUCTS ${other_SRC}
COMMAND shiboken2 --project-file=${CMAKE_CURRENT_BINARY_DIR}/other-binding.txt ${GENERATOR_EXTRA_FLAGS}
DEPENDS ${other_TYPESYSTEM} ${CMAKE_CURRENT_SOURCE_DIR}/global.h shiboken2
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
COMMENT "Running generator for 'other' test binding..."
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}
                    ${libother_SOURCE_DIR}
                    ${libsample_SOURCE_DIR}
                    ${libsample_SOURCE_DIR}/..
                    ${sample_BINARY_DIR}
                    ${sample_BINARY_DIR}/sample
                    ${libshiboken_SOURCE_DIR}
                    ${libshiboken_BINARY_DIR})
add_library(other MODULE ${other_SRC})
set_property(TARGET other PROPERTY PREFIX "")
set_property(TARGET other PROPERTY OUTPUT_NAME "other${PYTHON_EXTENSION_SUFFIX}")

if(WIN32)
    set_property(TARGET other PROPERTY SUFFIX ".pyd")
endif()
target_link_libraries(other
                      libother
                      libsample
                      libshiboken)

add_dependencies(other sample)
create_generator_target(other)

