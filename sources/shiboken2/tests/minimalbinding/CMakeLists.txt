project(minimal)

set(minimal_TYPESYSTEM
${CMAKE_CURRENT_SOURCE_DIR}/typesystem_minimal.xml
)

set(minimal_SRC
${CMAKE_CURRENT_BINARY_DIR}/minimal/minimal_module_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/minimal/obj_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/minimal/val_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/minimal/listuser_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/minimal/minbooluser_wrapper.cpp
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/minimal-binding.txt.in"
               "${CMAKE_CURRENT_BINARY_DIR}/minimal-binding.txt" @ONLY)

add_custom_command(
OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/mjb_rejected_classes.log"
BYPRODUCTS ${minimal_SRC}
COMMAND shiboken2 --project-file=${CMAKE_CURRENT_BINARY_DIR}/minimal-binding.txt ${GENERATOR_EXTRA_FLAGS}
DEPENDS ${minimal_TYPESYSTEM} ${CMAKE_CURRENT_SOURCE_DIR}/global.h shiboken2
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
COMMENT "Running generator for 'minimal' test binding..."
)

add_library(minimal MODULE ${minimal_SRC})
target_include_directories(minimal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(minimal PUBLIC libminimal libshiboken)
set_property(TARGET minimal PROPERTY PREFIX "")
set_property(TARGET minimal PROPERTY OUTPUT_NAME "minimal${PYTHON_EXTENSION_SUFFIX}")
if(WIN32)
    set_property(TARGET minimal PROPERTY SUFFIX ".pyd")
endif()

create_generator_target(minimal)
