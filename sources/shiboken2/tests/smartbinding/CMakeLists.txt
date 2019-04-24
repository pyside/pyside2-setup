project(smart)

set(smart_TYPESYSTEM
${CMAKE_CURRENT_SOURCE_DIR}/typesystem_smart.xml
)

set(smart_SRC
${CMAKE_CURRENT_BINARY_DIR}/smart/smart_module_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/smart/obj_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/smart/integer_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/smart/sharedptr_obj_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/smart/sharedptr_integer_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/smart/registry_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/smart/smart_integer2_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/smart/sharedptr_integer2_wrapper.cpp
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/smart-binding.txt.in"
               "${CMAKE_CURRENT_BINARY_DIR}/smart-binding.txt" @ONLY)

add_custom_command(
OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/mjb_rejected_classes.log"
BYPRODUCTS ${smart_SRC}
COMMAND shiboken2 --project-file=${CMAKE_CURRENT_BINARY_DIR}/smart-binding.txt ${GENERATOR_EXTRA_FLAGS}
DEPENDS ${smart_TYPESYSTEM} ${CMAKE_CURRENT_SOURCE_DIR}/global.h shiboken2
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
COMMENT "Running generator for 'smart' test binding..."
)

add_library(smart MODULE ${smart_SRC})
target_include_directories(smart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(smart PUBLIC libsmart libshiboken)
set_property(TARGET smart PROPERTY PREFIX "")
set_property(TARGET smart PROPERTY OUTPUT_NAME "smart${PYTHON_EXTENSION_SUFFIX}")

if(WIN32)
    set_property(TARGET smart PROPERTY SUFFIX ".pyd")
endif()

create_generator_target(smart)
