project(shibokenmodule)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/shibokenmodule.txt.in"
               "${CMAKE_CURRENT_BINARY_DIR}/shibokenmodule.txt" @ONLY)


set(sample_SRC
${CMAKE_CURRENT_BINARY_DIR}/shiboken2/shiboken2_module_wrapper.cpp
)

set(shibokenmodule_TYPESYSTEM
${CMAKE_CURRENT_SOURCE_DIR}/typesystem_shiboken.xml
)

add_custom_command(
OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/mjb_rejected_classes.log"
BYPRODUCTS ${sample_SRC}
# Note: shiboken2 is an executable target. By not specifying its explicit
# path, CMAKE figures it out, itself!
# This fixes an issue with Visual Studio, see https://github.com/PySide/shiboken2/pull/11
COMMAND shiboken2 --project-file=${CMAKE_CURRENT_BINARY_DIR}/shibokenmodule.txt ${GENERATOR_EXTRA_FLAGS}
DEPENDS ${shibokenmodule_TYPESYSTEM}
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
COMMENT "Running generator for 'shiboken2'..."
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}
                    ${libshiboken_SOURCE_DIR}
                    ${libshiboken_BINARY_DIR})
add_library(shibokenmodule MODULE ${sample_SRC})
set_property(TARGET shibokenmodule PROPERTY PREFIX "")
set_property(TARGET shibokenmodule PROPERTY OUTPUT_NAME "shiboken2${PYTHON_EXTENSION_SUFFIX}")

if(WIN32)
    set_property(TARGET shibokenmodule PROPERTY SUFFIX ".pyd")
endif()
target_link_libraries(shibokenmodule
                      libshiboken)

add_dependencies(shibokenmodule shiboken2)
create_generator_target(shibokenmodule)

install(TARGETS shibokenmodule DESTINATION ${PYTHON_SITE_PACKAGES}/shiboken2)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/_config.py.in"
               "${CMAKE_CURRENT_BINARY_DIR}/_config.py" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/_config.py"
        DESTINATION "${PYTHON_SITE_PACKAGES}/shiboken2")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
               "${CMAKE_CURRENT_BINARY_DIR}/__init__.py" @ONLY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/__init__.py"
               "${CMAKE_CURRENT_BINARY_DIR}/support/__init__.py" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/signature/__init__.py"
               "${CMAKE_CURRENT_BINARY_DIR}/support/signature/__init__.py" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/signature/errorhandler.py"
               "${CMAKE_CURRENT_BINARY_DIR}/support/signature/errorhandler.py" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/signature/layout.py"
               "${CMAKE_CURRENT_BINARY_DIR}/support/signature/layout.py" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/signature/loader.py"
               "${CMAKE_CURRENT_BINARY_DIR}/support/signature/loader.py" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/signature/mapping.py"
               "${CMAKE_CURRENT_BINARY_DIR}/support/signature/mapping.py" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/signature/parser.py"
               "${CMAKE_CURRENT_BINARY_DIR}/support/signature/parser.py" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/signature/lib/__init__.py"
               "${CMAKE_CURRENT_BINARY_DIR}/support/signature/lib/__init__.py" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/signature/lib/enum_sig.py"
               "${CMAKE_CURRENT_BINARY_DIR}/support/signature/lib/enum_sig.py" COPYONLY)
if (PYTHON_VERSION_MAJOR EQUAL 3)
else()
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/signature/backport_inspect.py"
                   "${CMAKE_CURRENT_BINARY_DIR}/support/signature/backport_inspect.py" COPYONLY)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/support/signature/typing27.py"
                   "${CMAKE_CURRENT_BINARY_DIR}/support/signature/typing27.py" COPYONLY)
endif()
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/support"
        DESTINATION "${PYTHON_SITE_PACKAGES}/shiboken2")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/__init__.py"
       DESTINATION "${PYTHON_SITE_PACKAGES}/shiboken2")

# Use absolute path instead of relative path, to avoid ninja build errors due to
# duplicate file dependency inconsistency.
set(shiboken_version_relative_path "${CMAKE_CURRENT_SOURCE_DIR}/../shiboken_version.py")
get_filename_component(shiboken_version_path ${shiboken_version_relative_path} ABSOLUTE)
configure_file("${shiboken_version_path}"
               "${CMAKE_CURRENT_BINARY_DIR}/_git_shiboken_module_version.py" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/_git_shiboken_module_version.py"
        DESTINATION "${PYTHON_SITE_PACKAGES}/shiboken2")
