cmake_minimum_required(VERSION 3.16)

if(BUILD_TESTS)
    find_package(Qt${QT_MAJOR_VERSION}Test 5.12 REQUIRED)
endif()

add_subdirectory(libminimal)
if(NOT DEFINED MINIMAL_TESTS)
    add_subdirectory(libsample)
    add_subdirectory(libsmart)
    add_subdirectory(libother)
endif()

if(WIN32 OR DEFINED AVOID_PROTECTED_HACK)
    message(STATUS "Tests will be generated avoiding the protected hack!")
    set(GENERATOR_EXTRA_FLAGS --avoid-protected-hack)
else()
    message(STATUS "Tests will be generated using the protected hack!")
    set(GENERATOR_EXTRA_FLAGS )
endif()

add_subdirectory(minimalbinding)
if(NOT DEFINED MINIMAL_TESTS)
    add_subdirectory(samplebinding)
    add_subdirectory(smartbinding)
    add_subdirectory(otherbinding)
endif()

if(DEFINED MINIMAL_TESTS)
    file(GLOB TEST_FILES minimalbinding/*_test.py)
else()
    file(GLOB TEST_FILES minimalbinding/*_test.py
                         samplebinding/*_test.py
                         otherbinding/*_test.py
                         smartbinding/*_test.py
                         shibokenmodule/*_test.py)
endif()
list(SORT TEST_FILES)

set(test_blacklist "")

find_package(PythonInterp REQUIRED)
if(NOT CTEST_TESTING_TIMEOUT)
    set(CTEST_TESTING_TIMEOUT 60)
endif()

get_filename_component(BUILD_DIR "${libminimal_BINARY_DIR}" DIRECTORY)
get_filename_component(BUILD_DIR "${BUILD_DIR}" DIRECTORY)
get_filename_component(BUILD_DIR "${BUILD_DIR}" DIRECTORY)
foreach(test_file ${TEST_FILES})
    string(REGEX MATCH "/([^/]+)(binding|module)/([^/]+)_test.py" tmp ${test_file})
    set(test_name "${CMAKE_MATCH_1}_${CMAKE_MATCH_3}")
    list(FIND test_blacklist ${test_name} expect_fail)
    add_test(${test_name} ${PYTHON_EXECUTABLE} ${test_file})
    set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "BUILD_DIR=${BUILD_DIR}")
    set_tests_properties(${test_name} PROPERTIES TIMEOUT ${CTEST_TESTING_TIMEOUT})
    if (${expect_fail} GREATER -1)
        set_tests_properties(${test_name} PROPERTIES WILL_FAIL TRUE)
    endif()
endforeach()

add_subdirectory(dumpcodemodel)

# FIXME Skipped until add an option to choose the generator
# add_subdirectory(test_generator)

if (NOT APIEXTRACTOR_DOCSTRINGS_DISABLED)
#     project(sphinxtabletest)
#
#     # TODO
#     set(sphinxtabletest_SRC sphinxtabletest.cpp)
#     qt4_automoc(${sphinxtabletest_SRC})
#
#     include_directories(${QT_INCLUDE_DIR}
#                         ${QT_QTCORE_INCLUDE_DIR}
#                         ${CMAKE_CURRENT_BINARY_DIR}
#                         ${qtdoc_generator_SOURCE_DIR})
#
#     add_executable(sphinxtabletest ${sphinxtabletest_SRC})
#
#     target_link_libraries(sphinxtabletest
#                         ${QT_QTTEST_LIBRARY}
#                         ${APIEXTRACTOR_LIBRARY}
#                         qtdoc_generator
#                         genrunner)
#
#     add_test("sphinxtable" sphinxtabletest)
#     if (INSTALL_TESTS)
#         install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/sphinxtabletest DESTINATION ${TEST_INSTALL_DIR})
#     endif()
endif()
