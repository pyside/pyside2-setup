project(shibokengenerator)

find_package(Qt5Xml)
find_package(Qt5XmlPatterns)

set(shiboken2_SRC
generator.cpp
shiboken2/cppgenerator.cpp
shiboken2/headergenerator.cpp
shiboken2/overloaddata.cpp
shiboken2/shibokengenerator.cpp
main.cpp
)

if (NOT DISABLE_DOCSTRINGS)
    set(shiboken2_SRC ${shiboken2_SRC} qtdoc/qtdocgenerator.cpp)
    add_definitions(-DDOCSTRINGS_ENABLED)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/shiboken2
                    ${CMAKE_CURRENT_SOURCE_DIR}/qtdoc
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${apiextractor_SOURCE_DIR}
                    ${Qt5Core_INCLUDE_DIRS}
                    ${Qt5XmlPatterns_INCLUDE_DIRS}
                    )

add_executable(shiboken2 ${shiboken2_SRC})
add_dependencies(shiboken2 apiextractor)
set_target_properties(shiboken2 PROPERTIES OUTPUT_NAME shiboken2${shiboken2_SUFFIX})
target_link_libraries(shiboken2
                      apiextractor
                      ${Qt5Core_LIBRARIES}
                      ${Qt5XmlPatterns_LIBRARIES}
                      )

configure_file(shibokenconfig.h.in "${CMAKE_CURRENT_BINARY_DIR}/shibokenconfig.h" @ONLY)

install(TARGETS shiboken2 DESTINATION bin)

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
