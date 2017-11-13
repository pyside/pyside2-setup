if(CMAKE_VERSION VERSION_LESS 2.8)
    message("CMake version greater than 2.8 necessary to run tests")
else()
    if(NOT CTEST_TESTING_TIMEOUT)
        set(CTEST_TESTING_TIMEOUT 60)
    endif()

   # tests/QtWidgets/qstandarditemmodel_test.py needs shiboken2
   if(WIN32)
        set(TEST_PYTHONPATH "${CMAKE_BINARY_DIR};${CMAKE_SOURCE_DIR}/tests/util;${CMAKE_BINARY_DIR}/tests/pysidetest;${CMAKE_BINARY_DIR}/../shiboken2/shibokenmodule;$ENV{PYTHONPATH}")
        set(TEST_LIBRARY_PATH   "${libpyside_BINARY_DIR};${pysidetest_BINARY_DIR};${SHIBOKEN_INCLUDE_DIR}/../../bin;$ENV{PATH}")
        set(LIBRARY_PATH_VAR    "PATH")
        string(REPLACE "\\" "/" TEST_PYTHONPATH "${TEST_PYTHONPATH}")
        string(REPLACE "\\" "/" TEST_LIBRARY_PATH "${TEST_LIBRARY_PATH}")

        string(REPLACE ";" "\\;" TEST_PYTHONPATH "${TEST_PYTHONPATH}")
        string(REPLACE ";" "\\;" TEST_LIBRARY_PATH "${TEST_LIBRARY_PATH}")
    else()
        set(TEST_PYTHONPATH "${CMAKE_BINARY_DIR}:${CMAKE_SOURCE_DIR}/tests/util:${CMAKE_BINARY_DIR}/tests/pysidetest:${CMAKE_BINARY_DIR}/../shiboken2/shibokenmodule:$ENV{PYTHONPATH}")
        set(TEST_LIBRARY_PATH   "${libpyside_BINARY_DIR}:${pysidetest_BINARY_DIR}:$ENV{LD_LIBRARY_PATH}")
        set(LIBRARY_PATH_VAR    "LD_LIBRARY_PATH")
    endif()


    macro(TEST_QT_MODULE var name)
      if(NOT DISABLE_${name} AND ${var})
          add_subdirectory(${name})
      endif()
    endmacro()

    macro(PYSIDE_TEST)
        string(REGEX MATCH "/([^/]+)//?([^/]+)\\.py" foo "${CMAKE_CURRENT_SOURCE_DIR}/${ARGV0}" )
        set(TEST_NAME "${CMAKE_MATCH_1}_${CMAKE_MATCH_2}")
        if (${ARGC} EQUAL 1)
            set(EXPECT_TO_FAIL 0)
        elseif(${ARGC} EQUAL 2)
            set(EXPECT_TO_FAIL ${ARGV1})
        else()
            message(WARNING "Invalid call of macro PYSIDE_TEST")
        endif()
        set(TEST_CMD ${XVFB_EXEC} ${SHIBOKEN_PYTHON_INTERPRETER} "${CMAKE_CURRENT_SOURCE_DIR}/${ARGV0}")
        add_test(${TEST_NAME} ${TEST_CMD})
        set_tests_properties(${TEST_NAME} PROPERTIES
                             TIMEOUT ${CTEST_TESTING_TIMEOUT}
                             WILL_FAIL ${EXPECT_TO_FAIL}
                             ENVIRONMENT "PYTHONPATH=${TEST_PYTHONPATH};${LIBRARY_PATH_VAR}=${TEST_LIBRARY_PATH};PYSIDE_DISABLE_INTERNAL_QT_CONF=1")
    endmacro()

    add_subdirectory(pysidetest)
    add_subdirectory(registry)
    add_subdirectory(signals)

    foreach(shortname IN LISTS all_module_shortnames)
        message(STATUS "preparing tests for module 'Qt${shortname}'")
        TEST_QT_MODULE(Qt5${shortname}_FOUND Qt${shortname})
    endforeach()

    #platform specific
    if (ENABLE_MAC)
        add_subdirectory(mac)
    endif ()
endif()
