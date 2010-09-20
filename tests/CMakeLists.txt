add_subdirectory(libsample)
add_subdirectory(libother)

# Detect if the python libs were compiled in debug mode
execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "from distutils import sysconfig; \\
        print sysconfig.get_config_var('Py_DEBUG')"
    OUTPUT_VARIABLE PY_DEBUG
    OUTPUT_STRIP_TRAILING_WHITESPACE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(NOT PYTHON_DEBUG_LIBRARIES)
        message(FATAL_ERROR "Python debug library not found. Try compile shiboken with -DCMAKE_BUILD_TYPE=Release")
    endif()
    if(NOT PY_DEBUG)
        message(WARNING "Compiling shiboken with debug enabled, but the python executable was not compiled with debug support.")
    else()
        add_definitions("-DPy_DEBUG")
    endif()
    set(SBK_PYTHON_LIBRARIES ${PYTHON_DEBUG_LIBRARIES})
else()
    set(SBK_PYTHON_LIBRARIES ${PYTHON_LIBRARIES})
    add_definitions("-DNDEBUG")
endif()

add_subdirectory(samplebinding)
add_subdirectory(otherbinding)

file(GLOB TEST_FILES samplebinding/*_test.py otherbinding/*_test.py)

set(test_blacklist "")

find_package(PythonInterp REQUIRED)
if(NOT CTEST_TESTING_TIMEOUT)
    set(CTEST_TESTING_TIMEOUT 60)
endif()

if(CMAKE_VERSION VERSION_LESS 2.8)
    message("CMake version greater than 2.8 necessary to run tests")
else()
    if(WIN32)
        set(TEST_PYTHONPATH     "${sample_BINARY_DIR};${other_BINARY_DIR}")
        set(TEST_LIBRARY_PATH   "$ENV{PATH};${libsample_BINARY_DIR};${libother_BINARY_DIR};${libshiboken_BINARY_DIR}")
        set(LIBRARY_PATH_VAR    "PATH")
	    string(REPLACE "\\" "/" TEST_PYTHONPATH "${TEST_PYTHONPATH}")
	    string(REPLACE "\\" "/" TEST_LIBRARY_PATH "${TEST_LIBRARY_PATH}")

	    string(REPLACE ";" "\\;" TEST_PYTHONPATH "${TEST_PYTHONPATH}")
	    string(REPLACE ";" "\\;" TEST_LIBRARY_PATH "${TEST_LIBRARY_PATH}")
    else()
        set(TEST_PYTHONPATH     "${sample_BINARY_DIR}:${other_BINARY_DIR}")
        set(TEST_LIBRARY_PATH   "$ENV{LD_LIBRARY_PATH}:${libsample_BINARY_DIR}:${libother_BINARY_DIR}:${libshiboken_BINARY_DIR}")
        set(LIBRARY_PATH_VAR    "LD_LIBRARY_PATH")
    endif()

    foreach(test_file ${TEST_FILES})
        string(REGEX MATCH "/([^/]+)_test.py" test_name ${test_file})
        list(FIND test_blacklist ${CMAKE_MATCH_1} expect_fail)
        add_test(${CMAKE_MATCH_1} ${PYTHON_EXECUTABLE} ${test_file})
        set_tests_properties(${CMAKE_MATCH_1} PROPERTIES ENVIRONMENT "PYTHONPATH=${TEST_PYTHONPATH};${LIBRARY_PATH_VAR}=${TEST_LIBRARY_PATH}")
        set_tests_properties(${CMAKE_MATCH_1} PROPERTIES TIMEOUT ${CTEST_TESTING_TIMEOUT})
        if (${expect_fail} GREATER -1)
            set_tests_properties(${CMAKE_MATCH_1} PROPERTIES WILL_FAIL TRUE)
        endif()
    endforeach()
endif()
