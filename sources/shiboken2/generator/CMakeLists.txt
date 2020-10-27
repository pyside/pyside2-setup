project(shibokengenerator)

set(shiboken2_SRC
generator.cpp
shiboken/cppgenerator.cpp
shiboken/headergenerator.cpp
shiboken/overloaddata.cpp
shiboken/shibokengenerator.cpp
main.cpp
)

add_executable(shiboken2 ${shiboken2_SRC})
add_executable(Shiboken2::shiboken2 ALIAS shiboken2)
add_dependencies(shiboken2 apiextractor)
set_target_properties(shiboken2 PROPERTIES OUTPUT_NAME shiboken2${shiboken2_SUFFIX})
target_include_directories(shiboken2 PRIVATE
                    ${CMAKE_CURRENT_SOURCE_DIR}/shiboken
                    ${CMAKE_CURRENT_SOURCE_DIR}/qtdoc
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${apiextractor_SOURCE_DIR}
                    )
target_link_libraries(shiboken2 apiextractor Qt${QT_MAJOR_VERSION}::Core)
if (NOT DISABLE_DOCSTRINGS)
    target_sources(shiboken2 PRIVATE qtdoc/qtdocgenerator.cpp)
    target_compile_definitions(shiboken2 PUBLIC DOCSTRINGS_ENABLED)
endif()

configure_file(shibokenconfig.h.in "${CMAKE_CURRENT_BINARY_DIR}/shibokenconfig.h" @ONLY)

install(TARGETS shiboken2
        EXPORT Shiboken2Targets
        DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")

set(shiboken_generator_package_name "shiboken2_generator")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/_config.py.in"
               "${CMAKE_CURRENT_BINARY_DIR}/_config.py" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/_config.py"
        DESTINATION "${PYTHON_SITE_PACKAGES}/${shiboken_generator_package_name}")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
               "${CMAKE_CURRENT_BINARY_DIR}/__init__.py" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/__init__.py"
       DESTINATION "${PYTHON_SITE_PACKAGES}/${shiboken_generator_package_name}")

# shiboken2 setuptools entry point
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../shiboken_tool.py
        DESTINATION bin
        PERMISSIONS
        OWNER_EXECUTE OWNER_WRITE OWNER_READ
        GROUP_EXECUTE GROUP_READ
        WORLD_EXECUTE WORLD_READ)

# Use absolute path instead of relative path, to avoid ninja build errors due to
# duplicate file dependency inconsistency.
set(shiboken_version_relative_path "${CMAKE_CURRENT_SOURCE_DIR}/../shiboken_version.py")
get_filename_component(shiboken_version_path ${shiboken_version_relative_path} ABSOLUTE)
configure_file("${shiboken_version_path}"
               "${CMAKE_CURRENT_BINARY_DIR}/_git_shiboken_generator_version.py" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/_git_shiboken_generator_version.py"
        DESTINATION "${PYTHON_SITE_PACKAGES}/${shiboken_generator_package_name}")
