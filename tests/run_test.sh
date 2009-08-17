#!/usr/bin/python

# This is a nasty workaround of a CTest limitation
# of setting the environment variables for the test.


LIB_PATH=$LD_LIBRARY_PATH:$1
PYTHON_PATH=$PYTHONPATH:$2
PYTHON_EXEC=$3
TEST_FILE=$4

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LIB_PATH PYTHONPATH=$PYTHON_PATH $PYTHON_EXEC $TEST_FILE

