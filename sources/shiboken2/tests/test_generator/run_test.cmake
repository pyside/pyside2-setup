# The tests are run through this script due to a limitation
# on versions of CMake lesser than 2.8, that prevent setting
# environment variables for tests from working.

set(ENV{PATH} "${ENV_PATH}")
set(ENV{QT_PLUGIN_PATH} "${ENV_QT_PLUGIN_PATH}")
execute_process(COMMAND ${TEST} WORKING_DIRECTORY "${WORKDIR}" RESULT_VARIABLE OK)

if(NOT OK EQUAL 0)
    message(SEND_ERROR "${TEST} failed!")
endif()
