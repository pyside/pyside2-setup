project(test_generator)

set(dummy_generator_SRC dummygenerator.cpp)
add_library(dummy_generator SHARED ${dummy_generator_SRC})
target_link_libraries(dummy_generator ${APIEXTRACTOR_LIBRARY} ${QT_QTCORE_LIBRARY} genrunner)
set_property(TARGET dummy_generator PROPERTY PREFIX "")

add_executable(dummygenerator main.cpp)
set(DUMMYGENERATOR_EXECUTABLE dummygenerator${generator_SUFFIX})
set_target_properties(dummygenerator PROPERTIES OUTPUT_NAME ${DUMMYGENERATOR_EXECUTABLE})
target_link_libraries(dummygenerator ${Qt5Core_LIBRARIES})

configure_file(dummygentestconfig.h.in "${CMAKE_CURRENT_BINARY_DIR}/dummygentestconfig.h" @ONLY)

get_filename_component(APIEXTRACTOR_LIBRARY_DIRS ${APIEXTRACTOR_LIBRARY} PATH)
if(WIN32)
    set(PATH_SEP ";")
    find_program(APIEXTRACTOR_BINARY apiextractor.dll HINTS ${APIEXTRACTOR_LIBRARY_DIRS})
    get_filename_component(APIEXTRACTOR_BINARY_DIR ${APIEXTRACTOR_BINARY} PATH)
    set(APIEXTRACTOR_LIBRARY_DIRS "${APIEXTRACTOR_LIBRARY_DIRS}${PATH_SEP}${APIEXTRACTOR_BINARY_DIR}")
else()
    set(PATH_SEP ":")
endif()

set(ENV_PATH "${generatorrunner_BINARY_DIR}${PATH_SEP}${CMAKE_CURRENT_BINARY_DIR}${PATH_SEP}$ENV{PATH}${PATH_SEP}${APIEXTRACTOR_LIBRARY_DIRS}")
set(ENV_QT_PLUGIN_PATH "${CMAKE_CURRENT_BINARY_DIR}${PATH_SEP}$ENV{QT_PLUGIN_PATH}")
if(WIN32)
    string(REPLACE "\\;" ";" ENV_PATH "${ENV_PATH}")
    string(REPLACE ";" "\\;" ENV_PATH "${ENV_PATH}")
    string(REPLACE "\\;" ";" ENV_QT_PLUGIN_PATH "${ENV_QT_PLUGIN_PATH}")
    string(REPLACE ";" "\\;" ENV_QT_PLUGIN_PATH "${ENV_QT_PLUGIN_PATH}")
endif()

macro(m_add_test testname)
    if(CMAKE_VERSION VERSION_LESS 2.8)
        add_test(${testname} ${CMAKE_COMMAND} -DTEST=${testname}
                 -DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}
                 -DENV_PATH=${ENV_PATH} -DENV_QT_PLUGIN_PATH=${ENV_QT_PLUGIN_PATH}
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_test.cmake)
    else()
        add_test(${testname} ${testname})
        set_property(TEST ${testname} PROPERTY ENVIRONMENT "PATH=${ENV_PATH}" "QT_PLUGIN_PATH=${ENV_QT_PLUGIN_PATH}")
    endif()
endmacro()

macro(declare_test testname)
    qt4_automoc("${testname}.cpp")
    add_executable(${testname} "${testname}.cpp")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(${testname}
                          ${QT_QTTEST_LIBRARY}
                          ${QT_QTCORE_LIBRARY}
                          ${Qt5Test_LIBRARIES}
                          ${Qt5Core_LIBRARIES}
                          )
    m_add_test(${testname})
endmacro(declare_test testname)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/test_global.h"
               "${CMAKE_CURRENT_BINARY_DIR}/test_global.h" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/test_typesystem.xml"
               "${CMAKE_CURRENT_BINARY_DIR}/test_typesystem.xml" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/dummygentest-project.txt.in"
               "${CMAKE_CURRENT_BINARY_DIR}/dummygentest-project.txt" @ONLY)
declare_test(dummygentest)

add_dependencies(dummygenerator generatorrunner)

