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
      module_static_sources)
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

    if (NOT EXISTS ${typesystem_name})
        set(typesystem_path ${CMAKE_CURRENT_SOURCE_DIR}/typesystem_${_module}.xml)
    else()
        set(typesystem_path ${typesystem_name})
    endif()

    # check for class files that were commented away.
    if(DEFINED ${module_sources}_skipped_files)
        if(DEFINED PYTHON3_EXECUTABLE)
            set(_python_interpreter "${PYTHON3_EXECUTABLE}")
        else()
            set(_python_interpreter "${PYTHON_EXECUTABLE}")
        endif()
        if(NOT _python_interpreter)
            message(FATAL_ERROR "*** we need a python interpreter for postprocessing!")
        endif()
        set(_python_postprocessor "${_python_interpreter}" "${CMAKE_CURRENT_BINARY_DIR}/filter_init.py")
    else()
        set(_python_postprocessor "")
    endif()

    # Create typesystem XML dependencies list, so that whenever they change, shiboken is invoked
    # automatically.
    # First add the main file.
    set(total_type_system_files ${typesystem_path})

    # Transform the path separator list back into a cmake list (so from a:b:c to a;b;c)
    unmake_path(list_of_paths ${${module_typesystem_path}})

    # Collect all XML files, in each given path, and append them to the final total list.
    foreach(type_system_files_path ${list_of_paths})
        set(glob_expression "${type_system_files_path}/*.xml")
        file(GLOB type_system_files ${glob_expression})
        set(total_type_system_files ${total_type_system_files} ${type_system_files})
    endforeach(type_system_files_path)

    # Remove any possible duplicates.
    list(REMOVE_DUPLICATES total_type_system_files)

    # Contains include directories to pass to shiboken's preprocessor.
    set(shiboken_include_dirs ${pyside2_SOURCE_DIR}${PATH_SEP}${QT_INCLUDE_DIR})
    set(shiboken_framework_include_dirs_option "")
    if(CMAKE_HOST_APPLE)
        set(shiboken_framework_include_dirs "${QT_FRAMEWORK_INCLUDE_DIR}")
        # On macOS, provide the framework paths for OpenGL headers.
        set(shiboken_framework_include_dirs ${shiboken_framework_include_dirs} ${CMAKE_SYSTEM_FRAMEWORK_PATH})
        make_path(shiboken_framework_include_dirs ${shiboken_framework_include_dirs})
        set(shiboken_framework_include_dirs_option "--framework-include-paths=${shiboken_framework_include_dirs}")
    endif()

    # Transform the path separators into something shiboken understands.
    make_path(shiboken_include_dirs ${shiboken_include_dirs})

    add_custom_command(OUTPUT ${${module_sources}}
                        COMMAND "${SHIBOKEN_BINARY}" ${GENERATOR_EXTRA_FLAGS}
                        ${pyside2_BINARY_DIR}/pyside2_global.h
                        --include-paths=${shiboken_include_dirs}
                        ${shiboken_framework_include_dirs_option}
                        --typesystem-paths=${pyside2_SOURCE_DIR}${PATH_SEP}${${module_typesystem_path}}
                        --output-directory=${CMAKE_CURRENT_BINARY_DIR}
                        --license-file=${CMAKE_CURRENT_SOURCE_DIR}/../licensecomment.txt
                        ${typesystem_path}
                        --api-version=${SUPPORTED_QT_VERSION}
                        --drop-type-entries="${dropped_entries}"
                        COMMAND ${_python_postprocessor}
                        DEPENDS ${total_type_system_files}
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
    endif()

    target_link_libraries(${module_name} ${${module_libraries}})
    if(${module_deps})
        add_dependencies(${module_name} ${${module_deps}})
    endif()

    # install
    install(TARGETS ${module_name} LIBRARY DESTINATION ${SITE_PACKAGE}/PySide2)
    string(TOLOWER ${module_name} lower_module_name)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/PySide2/${module_name}/pyside2_${lower_module_name}_python.h
            DESTINATION include/PySide2${pyside2_SUFFIX}/${module_name}/)
    file(GLOB typesystem_files ${CMAKE_CURRENT_SOURCE_DIR}/typesystem_*.xml ${typesystem_path})
    install(FILES ${typesystem_files} DESTINATION share/PySide2${pyside2_SUFFIX}/typesystems)
endmacro()

#macro(check_qt_class_with_namespace module namespace class optional_source_files dropped_entries [namespace] [module])
macro(check_qt_class module class optional_source_files dropped_entries)
    if (${ARGC} GREATER 4)
        set (namespace ${ARGV4})
        string(TOLOWER ${namespace} _namespace)
    else ()
        set (namespace "")
    endif ()
    if (${ARGC} GREATER 5)
        set (include_file ${ARGV5})
    else ()
        set (include_file ${class})
    endif ()
    string(TOLOWER ${class} _class)
    # Remove the "Qt" prefix.
    string(SUBSTRING ${module} 2 -1 _module_no_qt_prefix)
    if (_namespace)
        set(_cppfile ${CMAKE_CURRENT_BINARY_DIR}/PySide2/${module}/${_namespace}_${_class}_wrapper.cpp)
    else ()
        set(_cppfile ${CMAKE_CURRENT_BINARY_DIR}/PySide2/${module}/${_class}_wrapper.cpp)
    endif ()
    if (DEFINED PYSIDE_${class})
        if (PYSIDE_${class})
            list(APPEND ${optional_source_files} ${_cppfile})
        else()
            list(APPEND ${dropped_entries} PySide2.${module}.${class})
        endif()
    else()
        if (NOT ${namespace} STREQUAL "" )
            set (NAMESPACE_USE "using namespace ${namespace};")
        else ()
            set (NAMESPACE_USE "")
        endif ()
        set(SRC_FILE ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/test${class}.cxx)
        file(WRITE ${SRC_FILE}
             "#include <${include_file}>\n"
             "${NAMESPACE_USE}\n"
             "int main() { sizeof(${class}); }\n"
        )
        try_compile(Q_WORKS ${CMAKE_BINARY_DIR}
                    ${SRC_FILE}
                    CMAKE_FLAGS
                        "-DINCLUDE_DIRECTORIES=${QT_INCLUDE_DIR};${Qt5${_module_no_qt_prefix}_INCLUDE_DIRS}"
                    OUTPUT_VARIABLE OUTPUT)
        file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeCheckQtClassTest.log ${OUTPUT})

        set("PYSIDE_${class}" ${Q_WORKS} CACHE STRING "Has ${class} class been found?")
        if(Q_WORKS)
            message(STATUS "Checking for ${class} in ${module} -- found")
            list(APPEND ${optional_source_files} ${_cppfile})
        else()
            message(STATUS "Checking for ${class} in ${module} -- not found")
            list(APPEND ${dropped_entries} PySide2.${module}.${class})
        endif()
    endif()
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

