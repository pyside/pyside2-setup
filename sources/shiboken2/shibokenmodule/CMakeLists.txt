project(shibokenmodule)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/shibokenmodule.txt.in"
               "${CMAKE_CURRENT_BINARY_DIR}/shibokenmodule.txt" @ONLY)


set(sample_SRC
${CMAKE_CURRENT_BINARY_DIR}/shiboken2/shiboken2_module_wrapper.cpp
)

add_custom_command(OUTPUT ${sample_SRC}
# Note: shiboken2 is an executable target. By not specifying its explicit
# path, CMAKE figures it out, itself!
# This fixes an issue with Visual Studio, see https://github.com/PySide/shiboken2/pull/11
COMMAND shiboken2 --project-file=${CMAKE_CURRENT_BINARY_DIR}/shibokenmodule.txt ${GENERATOR_EXTRA_FLAGS}
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
COMMENT "Running generator for 'shiboken2'..."
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}
                    ${SBK_PYTHON_INCLUDE_DIR}
                    ${libshiboken_SOURCE_DIR}
                    ${libshiboken_BINARY_DIR})
add_library(shibokenmodule MODULE ${sample_SRC})
set_property(TARGET shibokenmodule PROPERTY PREFIX "")
set_property(TARGET shibokenmodule PROPERTY OUTPUT_NAME "shiboken2")
if(WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_property(TARGET shibokenmodule PROPERTY OUTPUT_NAME "shiboken2_d")
    endif()
    set_property(TARGET shibokenmodule PROPERTY SUFFIX ".pyd")
endif()
target_link_libraries(shibokenmodule
                      ${SBK_PYTHON_LIBRARIES}
                      libshiboken)

add_dependencies(shibokenmodule shiboken2)

install(TARGETS shibokenmodule DESTINATION ${PYTHON_SITE_PACKAGES})