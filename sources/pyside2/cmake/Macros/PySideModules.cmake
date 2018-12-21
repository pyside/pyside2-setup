include(CMakeParseArguments)

# A version of cmake_parse_arguments that makes sure all arguments are processed and errors out
# with a message about ${type} having received unknown arguments.
macro(pyside_parse_all_arguments prefix type flags options multiopts)
    cmake_parse_arguments(${prefix} "${flags}" "${options}" "${multiopts}" ${ARGN})
    if(DEFINED ${prefix}_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown arguments were passed to ${type} (${${prefix}_UNPARSED_ARGUMENTS}).")
    endif()
endmacro()

macro(make_path varname)
   # accepts any number of path variables
   string(REPLACE ";" "${PATH_SEP}" ${varname} "${ARGN}")
endmacro()

macro(unmake_path varname)
   string(REPLACE "${PATH_SEP}" ";" ${varname} "${ARGN}")
endmacro()

# Sample usage
# create_pyside_module(NAME QtGui
#                      INCLUDE_DIRS QtGui_include_dirs
#                      LIBRARIES QtGui_libraries
#                      DEPS QtGui_deps
#                      TYPESYSTEM_PATH QtGui_SOURCE_DIR
#                      SOURCES QtGui_SRC
#                      STATIC_SOURCES QtGui_static_sources
#                      TYPESYSTEM_NAME ${QtGui_BINARY_DIR}/typesystem_gui.xml
#                      DROPPED_ENTRIES QtGui_DROPPED_ENTRIES
#                      GLUE_SOURCES QtGui_glue_sources)
macro(create_pyside_module)
    pyside_parse_all_arguments(
        "module" # Prefix
        "create_pyside_module" # Macro name
        "" # Flags
        "NAME;TYPESYSTEM_PATH;TYPESYSTEM_NAME" # Single value
        "INCLUDE_DIRS;LIBRARIES;DEPS;SOURCES;STATIC_SOURCES;DROPPED_ENTRIES;GLUE_SOURCES" # Multival
        ${ARGN} # Number of arguments given when the macros is called
        )

    if ("${module_NAME}" STREQUAL "")
        message(FATAL_ERROR "create_pyside_module needs a NAME value.")
    endif()
    if ("${module_INCLUDE_DIRS}" STREQUAL "")
        message(FATAL_ERROR "create_pyside_module needs at least one INCLUDE_DIRS value.")
    endif()
    if ("${module_TYPESYSTEM_PATH}" STREQUAL "")
        message(FATAL_ERROR "create_pyside_module needs a TYPESYSTEM_PATH value.")
    endif()
    if ("${module_SOURCES}" STREQUAL "")
        message(FATAL_ERROR "create_pyside_module needs at least one SOURCES value.")
    endif()

    string(TOLOWER ${module_NAME} _module)
    string(REGEX REPLACE ^qt "" _module ${_module})

    if(${module_DROPPED_ENTRIES})
        string(REPLACE ";" "\\;" dropped_entries "${${module_DROPPED_ENTRIES}}")
    else()
        set (dropped_entries "")
    endif()

    if(${module_GLUE_SOURCES})
        set (module_GLUE_SOURCES "${${module_GLUE_SOURCES}}")
    else()
        set (module_GLUE_SOURCES "")
    endif()

    if (NOT EXISTS ${module_TYPESYSTEM_NAME})
        set(typesystem_path ${CMAKE_CURRENT_SOURCE_DIR}/typesystem_${_module}.xml)
    else()
        set(typesystem_path ${module_TYPESYSTEM_NAME})
    endif()

    # Create typesystem XML dependencies list, so that whenever they change, shiboken is invoked
    # automatically.
    # First add the main file.
    set(total_type_system_files ${typesystem_path})

    get_filename_component(typesystem_root "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)

    set(deps ${module_NAME} ${${module_DEPS}})
    foreach(dep ${deps})
        set(glob_expression "${typesystem_root}/${dep}/*.xml")
        file(GLOB type_system_files ${glob_expression})
        set(total_type_system_files ${total_type_system_files} ${type_system_files})
    endforeach(dep)

    # Remove any possible duplicates.
    list(REMOVE_DUPLICATES total_type_system_files)

    # Contains include directories to pass to shiboken's preprocessor.
    set(shiboken_include_dirs ${pyside2_SOURCE_DIR}${PATH_SEP}${QT_INCLUDE_DIR})
    set(shiboken_framework_include_dirs_option "")
    if(CMAKE_HOST_APPLE)
        set(shiboken_framework_include_dirs "${QT_FRAMEWORK_INCLUDE_DIR}")
        make_path(shiboken_framework_include_dirs ${shiboken_framework_include_dirs})
        set(shiboken_framework_include_dirs_option "--framework-include-paths=${shiboken_framework_include_dirs}")
    endif()

    # Transform the path separators into something shiboken understands.
    make_path(shiboken_include_dirs ${shiboken_include_dirs})

    get_filename_component(pyside_binary_dir ${CMAKE_CURRENT_BINARY_DIR} DIRECTORY)

    # Install module glue files.
    string(TOLOWER ${module_NAME} lower_module_name)
    set(${module_NAME}_glue "${CMAKE_CURRENT_SOURCE_DIR}/../glue/${lower_module_name}.cpp")
    set(${module_name}_glue_dependency "")
    if(EXISTS ${${module_NAME}_glue})
        install(FILES ${${module_NAME}_glue} DESTINATION share/PySide2${pyside2_SUFFIX}/glue)
        set(${module_NAME}_glue_dependency ${${module_NAME}_glue})
    endif()

    # Install standalone glue files into typesystems subfolder, so that the resolved relative
    # paths remain correct.
    if (module_GLUE_SOURCES)
        install(FILES ${module_GLUE_SOURCES} DESTINATION share/PySide2${pyside2_SUFFIX}/typesystems/glue)
    endif()

    add_custom_command( OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/mjb_rejected_classes.log"
                        BYPRODUCTS ${${module_SOURCES}}
                        COMMAND "${SHIBOKEN_BINARY}" ${GENERATOR_EXTRA_FLAGS}
                        "${pyside2_BINARY_DIR}/${module_NAME}_global.h"
                        --include-paths=${shiboken_include_dirs}
                        ${shiboken_framework_include_dirs_option}
                        --typesystem-paths=${pyside_binary_dir}${PATH_SEP}${pyside2_SOURCE_DIR}${PATH_SEP}${${module_TYPESYSTEM_PATH}}
                        --output-directory=${CMAKE_CURRENT_BINARY_DIR}
                        --license-file=${CMAKE_CURRENT_SOURCE_DIR}/../licensecomment.txt
                        ${typesystem_path}
                        --api-version=${SUPPORTED_QT_VERSION}
                        --drop-type-entries="${dropped_entries}"
                        DEPENDS ${total_type_system_files}
                                ${module_GLUE_SOURCES}
                                ${${module_NAME}_glue_dependency}
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        COMMENT "Running generator for ${module_NAME}...")

    include_directories(${module_NAME} ${${module_INCLUDE_DIRS}} ${pyside2_SOURCE_DIR})
    add_library(${module_NAME} MODULE ${${module_SOURCES}}
                                      ${${module_STATIC_SOURCES}})
    set_target_properties(${module_NAME} PROPERTIES
                          PREFIX ""
                          OUTPUT_NAME "${module_NAME}${PYTHON_EXTENSION_SUFFIX}"
                          LIBRARY_OUTPUT_DIRECTORY ${pyside2_BINARY_DIR})
    if(WIN32)
        set_target_properties(${module_NAME} PROPERTIES SUFFIX ".pyd")
        # Sanitize windows.h as pulled by gl.h to prevent clashes with QAbstract3dAxis::min(), etc.
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX")
    endif()

    target_link_libraries(${module_NAME} ${${module_LIBRARIES}})
    if(${module_DEPS})
        add_dependencies(${module_NAME} ${${module_DEPS}})
    endif()
    create_generator_target(${module_NAME})

    # build type hinting stubs
    set(generate_pyi_options run --skip --sys-path "${CMAKE_BINARY_DIR}"
        "${CMAKE_BINARY_DIR}/../shiboken2/shibokenmodule"
        --lib-path "${CMAKE_BINARY_DIR}/libpyside"
        "${CMAKE_BINARY_DIR}/../shiboken2/libshiboken")
    if (QUIET_BUILD)
        list(APPEND generate_pyi_options "--quiet")
    endif()
    add_custom_command( TARGET ${module_NAME} POST_BUILD
                        COMMAND "${SHIBOKEN_PYTHON_INTERPRETER}"
                        "${CMAKE_CURRENT_SOURCE_DIR}/../support/generate_pyi.py" ${generate_pyi_options})
    # install
    install(TARGETS ${module_NAME} LIBRARY DESTINATION "${PYTHON_SITE_PACKAGES}/PySide2")

    file(GLOB hinting_stub_files RELATIVE "${CMAKE_CURRENT_BINARY_DIR}/PySide2" "${CMAKE_CURRENT_BINARY_DIR}/PySide2/*.pyi")
    install(FILES ${hinting_stub_files}
            DESTINATION "${PYTHON_SITE_PACKAGES}/PySide2")

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/PySide2/${module_NAME}/pyside2_${lower_module_name}_python.h
            DESTINATION include/PySide2${pyside2_SUFFIX}/${module_NAME}/)
    file(GLOB typesystem_files ${CMAKE_CURRENT_SOURCE_DIR}/typesystem_*.xml ${typesystem_path})

#   Copy typesystem files and remove module names from the <load-typesystem> element
#   so that it works in a flat directory:
#   <load-typesystem name="QtWidgets/typesystem_widgets.xml" ... ->
#   <load-typesystem name="typesystem_widgets.xml"
    foreach(typesystem_file ${typesystem_files})
        get_filename_component(typesystem_file_name "${typesystem_file}" NAME)
        file(READ "${typesystem_file}" typesystemXml)
        string(REGEX REPLACE "<load-typesystem name=\"[^/\"]+/" "<load-typesystem name=\"" typesystemXml "${typesystemXml}")
        set (typesystem_target_file "${CMAKE_CURRENT_BINARY_DIR}/PySide2/typesystems/${typesystem_file_name}")
        file(WRITE "${typesystem_target_file}" "${typesystemXml}")
        install(FILES "${typesystem_target_file}" DESTINATION share/PySide2${pyside2_SUFFIX}/typesystems)
    endforeach()
endmacro()

# Only add subdirectory if the associated Qt module is found.
# As a side effect, this macro now also defines the variable ${name}_GEN_DIR
# and must be called for every subproject.
macro(HAS_QT_MODULE var name)
    if (NOT DISABLE_${name} AND ${var})
        # we keep the PySide name here because this is compiled into shiboken
        set(${name}_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/${name}/PySide2/${name}
            CACHE INTERNAL "dir with generated source" FORCE)
        add_subdirectory(${name})
    else()
        # Used on documentation to skip modules
        set("if_${name}" "<!--" PARENT_SCOPE)
        set("end_${name}" "-->" PARENT_SCOPE)
    endif()
endmacro()

