add_subdirectory(libsample)
add_subdirectory(samplebinding)

file(GLOB TEST_FILES samplebinding/*_test.py)

foreach(test_file ${TEST_FILES})
    string(REGEX MATCH "/([^/]+)_test.py" test_name ${test_file})
    add_test(${CMAKE_MATCH_1} sh
             ${CMAKE_CURRENT_SOURCE_DIR}/run_test.sh
             "${libsample_BINARY_DIR}:${libshiboken_BINARY_DIR}"
             "${sample_BINARY_DIR}"
             ${PYTHON_EXECUTABLE}
             ${test_file})
    set_tests_properties(${CMAKE_MATCH_1} PROPERTIES TIMEOUT 5)

# Should set python path here
# Looks like it's fixed in 2.8:
# http://www.vtk.org/Bug/print_bug_page.php?bug_id=7885
endforeach(test_file ${TEST_FILES})

