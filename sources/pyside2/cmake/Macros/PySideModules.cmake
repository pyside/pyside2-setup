macro(make_path varname)
   # accepts any number of path variables
   string(REPLACE ";" "${PATH_SEP}" ${varname} "${ARGN}")
endmacro()

macro(unmake_path varname)
   string(REPLACE "${PATH_SEP}" ";" ${varname} "${ARGN}")
endmacro()

macro(create_pyside_module
      module_name
      module_include_dir
      module_libraries
      module_deps
      module_typesystem_path
      module_sources
      module_static_sources
      #module_typesystem_name
      #module_dropped_entries
      #module_glue_sources
      )
    string(TOLOWER ${module_name} _module)
    string(REGEX REPLACE ^qt "" _module ${_module})
    if(${ARGC} GREATER 7)
        set (typesystem_name ${ARGV7})
    else()
        set (typesystem_name "")
    endif()
    if(${ARGC} GREATER 8)
        string(REPLACE ";" "\\;" dropped_entries "${${ARGV8}}")
    else()
        set (dropped_entries "")
    endif()
    if(${ARGC} GREATER 9)
        set (glue_sources "${${ARGV9}}")
    else()
        set (glue_sources "")
    endif()

    if (NOT EXISTS ${typesystem_name})
        set(typesystem_path ${CMAKE_CURRENT_SOURCE_DIR}/typesystem_${_module}.xml)
    else()
        set(typesystem_path ${typesystem_name})
    endif()

    # Create typesystem XML dependencies list, so that whenever they change, shiboken is invoked
    # automatically.
    # First add the main file.
    set(total_type_system_files ${typesystem_path})

    get_filename_component(typesystem_root "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)

    set(deps ${module_name} ${${module_deps}})
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
    string(TOLOWER ${module_name} lower_module_name)
    set(${module_name}_glue "${CMAKE_CURRENT_SOURCE_DIR}/../glue/${lower_module_name}.cpp")
    set(${module_name}_glue_dependency "")
    if(EXISTS ${${module_name}_glue})
        install(FILES ${${module_name}_glue} DESTINATION share/PySide2${pyside2_SUFFIX}/glue)
        set(${module_name}_glue_dependency ${${module_name}_glue})
    endif()

    # Install standalone glue files into typesystems subfolder, so that the resolved relative
    # paths remain correct.
    if (glue_sources)
        install(FILES ${glue_sources} DESTINATION share/PySide2${pyside2_SUFFIX}/typesystems/glue)
    endif()

    add_custom_command( OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/mjb_rejected_classes.log"
                        BYPRODUCTS ${${module_sources}}
                        COMMAND "${SHIBOKEN_BINARY}" ${GENERATOR_EXTRA_FLAGS}
                        "${pyside2_BINARY_DIR}/${module_name}_global.h"
                        --include-paths=${shiboken_include_dirs}
                        ${shiboken_framework_include_dirs_option}
                        --typesystem-paths=${pyside_binary_dir}${PATH_SEP}${pyside2_SOURCE_DIR}${PATH_SEP}${${module_typesystem_path}}
                        --output-directory=${CMAKE_CURRENT_BINARY_DIR}
                        --license-file=${CMAKE_CURRENT_SOURCE_DIR}/../licensecomment.txt
                        ${typesystem_path}
                        --api-version=${SUPPORTED_QT_VERSION}
                        --drop-type-entries="${dropped_entries}"
                        DEPENDS ${total_type_system_files}
                                ${glue_sources}
                                ${${module_name}_glue_dependency}
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        COMMENT "Running generator for ${module_name}...")

    include_directories(${module_name} ${${module_include_dir}} ${pyside2_SOURCE_DIR})
    add_library(${module_name} MODULE ${${module_sources}} ${${module_static_sources}})
    set_target_properties(${module_name} PROPERTIES
                          PREFIX ""
                          OUTPUT_NAME "${module_name}${PYTHON_EXTENSION_SUFFIX}"
                          LIBRARY_OUTPUT_DIRECTORY ${pyside2_BINARY_DIR})
    if(WIN32)
        set_target_properties(${module_name} PROPERTIES SUFFIX ".pyd")
        # Sanitize windows.h as pulled by gl.h to prevent clashes with QAbstract3dAxis::min(), etc.
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX")
    endif()

    target_link_libraries(${module_name} ${${module_libraries}})
    if(${module_deps})
        add_dependencies(${module_name} ${${module_deps}})
    endif()
    create_generator_target(${module_name})

    # install
    install(TARGETS ${module_name} LIBRARY DESTINATION ${PYTHON_SITE_PACKAGES}/PySide2)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/PySide2/${module_name}/pyside2_${lower_module_name}_python.h
            DESTINATION include/PySide2${pyside2_SUFFIX}/${module_name}/)
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

