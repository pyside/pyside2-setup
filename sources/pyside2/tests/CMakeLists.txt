if(NOT CTEST_TESTING_TIMEOUT)
    set(CTEST_TESTING_TIMEOUT 60)
endif()

# BUILD_DIR and QT_DIR are used by init_paths.py for setting
# the path to the testbinding module
get_filename_component(BUILD_DIR "${CMAKE_BINARY_DIR}" DIRECTORY)
get_filename_component(BUILD_DIR "${CMAKE_BINARY_DIR}" DIRECTORY)
set(QT_DIR "${_qt5Core_install_prefix}")

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
    # QT_NO_GLIB=1 is required to avoid crashes on CI RHEL 6.6 machines.
    # See QTBUG-64716 for details.
    set_tests_properties(${TEST_NAME} PROPERTIES
                         TIMEOUT ${CTEST_TESTING_TIMEOUT}
                         WILL_FAIL ${EXPECT_TO_FAIL}
                         ENVIRONMENT "BUILD_DIR=${BUILD_DIR};QT_DIR=${QT_DIR};PYSIDE_DISABLE_INTERNAL_QT_CONF=1;QT_NO_GLIB=1")
endmacro()

if (NOT DISABLE_QtCore AND NOT DISABLE_QtGui AND NOT DISABLE_QtWidgets)
    add_subdirectory(pysidetest)
endif()
add_subdirectory(registry)
add_subdirectory(signals)
add_subdirectory(support)

foreach(shortname IN LISTS all_module_shortnames)
    message(STATUS "preparing tests for module 'Qt${shortname}'")
    TEST_QT_MODULE(Qt${QT_MAJOR_VERSION}${shortname}_FOUND Qt${shortname})
endforeach()

#platform specific
if (ENABLE_MAC)
    add_subdirectory(mac)
endif ()
