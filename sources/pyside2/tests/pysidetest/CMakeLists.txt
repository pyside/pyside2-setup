project(pysidetest)
project(testbinding)

cmake_minimum_required(VERSION 2.6)

set(QT_USE_QTCORE 1)
# no more supported: include(${QT_USE_FILE})
add_definitions(${Qt5Core_DEFINITIONS})
add_definitions(-DQT_SHARED)
add_definitions(-DRXX_ALLOCATOR_INIT_0)

find_package(Qt5Widgets)

set(pysidetest_SRC
testobject.cpp
testview.cpp
hiddenobject.cpp
)

set(pysidetest_MOC_HEADERS
testobject.h
testview.h
hiddenobject.h
)

qt5_wrap_cpp(pysidetest_MOC_SRC ${pysidetest_MOC_HEADERS})

set(testbinding_SRC
${CMAKE_CURRENT_BINARY_DIR}/testbinding/testobject_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/testbinding/intvalue_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/testbinding/pysidecpp_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/testbinding/pysidecpp_testobjectwithnamespace_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/testbinding/pysidecpp2_testobjectwithoutnamespace_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/testbinding/testview_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/testbinding/testbinding_module_wrapper.cpp
)

# Get per module include dirs.
# There are usually 3 paths there:
# ./qt/include/; ./qt/include/QtCore ; ./qt/mkspecs/linux-g++
# on framework build they are:
# ./qt/lib/QtCore.framework; ./qt/lib/QtCore.framework/Headers ; ./qt/mkspecs/macx-clang
# Thus we use the second direct path, which contains the actual header files.

list(GET Qt5Core_INCLUDE_DIRS 1 Qt5Core_DIRECT_INCLUDE_DIR)
list(GET Qt5Gui_INCLUDE_DIRS 1 Qt5Gui_DIRECT_INCLUDE_DIR)
list(GET Qt5Widgets_INCLUDE_DIRS 1 Qt5Widgets_DIRECT_INCLUDE_DIR)

# Adjust include headers paths for frameworks.
set(shiboken_framework_include_dirs_option "")
if(CMAKE_HOST_APPLE AND QtCore_is_framework)
    set(shiboken_framework_include_dirs "${QT_FRAMEWORK_INCLUDE_DIR}")
    set(shiboken_framework_include_dirs_option "--framework-include-paths=${shiboken_framework_include_dirs}")
endif()

make_path(testbinding_include_dirs  ${pyside2_BINARY_DIR}
                                    ${CMAKE_CURRENT_SOURCE_DIR}
                                    ${CMAKE_CURRENT_SOURCE_DIR}/../../PySide2
                                    ${CMAKE_CURRENT_SOURCE_DIR}/../../libpyside
                                    ${QT_INCLUDE_DIR}
                                    ${Qt5Core_DIRECT_INCLUDE_DIR}
                                    ${Qt5Gui_DIRECT_INCLUDE_DIR}
                                    ${Qt5Widgets_DIRECT_INCLUDE_DIR}
                                    )

make_path(testbinding_typesystem_path ${CMAKE_CURRENT_SOURCE_DIR}
                                      ${pyside2_SOURCE_DIR}
                                      ${QtCore_SOURCE_DIR} ${QtGui_SOURCE_DIR} ${QtWidgets_SOURCE_DIR}
                                      ${QtCore_BINARY_DIR} ${QtGui_BINARY_DIR} ${QtWidgets_BINARY_DIR}
                                      )

add_custom_command(OUTPUT ${testbinding_SRC}
COMMAND ${SHIBOKEN_BINARY} ${GENERATOR_EXTRA_FLAGS}
        ${CMAKE_CURRENT_SOURCE_DIR}/pysidetest_global.h
        --include-paths=${testbinding_include_dirs}
        ${shiboken_framework_include_dirs_option}
        --typesystem-paths=${testbinding_typesystem_path}
        --output-directory=${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/typesystem_pysidetest.xml
        --api-version=${SUPPORTED_QT_VERSION}
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
COMMENT "Running generator for test binding..."
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${Qt5Core_INCLUDE_DIRS}
                    ${Qt5Gui_INCLUDE_DIRS}
                    ${Qt5Widgets_INCLUDE_DIRS}
                    ${SHIBOKEN_INCLUDE_DIR}
                    ${pyside2_SOURCE_DIR}
                    ${QtCore_GEN_DIR}
                    ${QtGui_GEN_DIR}
                    ${QtWidgets_GEN_DIR}
                    ${libpyside_SOURCE_DIR}
                    ${SHIBOKEN_PYTHON_INCLUDE_DIR})

add_library(pysidetest SHARED ${pysidetest_SRC} ${pysidetest_MOC_SRC})
target_link_libraries(pysidetest
                      ${Qt5Core_LIBRARIES}
                      ${Qt5Gui_LIBRARIES}
                      ${Qt5Widgets_LIBRARIES}
                      )

add_library(testbinding MODULE ${testbinding_SRC})
set_property(TARGET testbinding PROPERTY PREFIX "")
if(WIN32)
    set_target_properties(testbinding PROPERTIES SUFFIX ".pyd")
endif()
target_link_libraries(testbinding
                      pysidetest
                      pyside2
                      ${PYTHON_LIBRARIES}
                      ${SHIBOKEN_LIBRARY}
                      ${Qt5Core_LIBRARIES}
                      ${Qt5Gui_LIBRARIES}
                      ${Qt5Widgets_LIBRARIES}
                      ${SBK_PYTHON_LIBRARIES})

add_dependencies(testbinding pyside2 QtCore QtGui QtWidgets libpyside pysidetest)

PYSIDE_TEST(decoratedslot_test.py)
PYSIDE_TEST(delegatecreateseditor_test.py)
PYSIDE_TEST(enum_test.py)
PYSIDE_TEST(homonymoussignalandmethod_test.py)
PYSIDE_TEST(list_signal_test.py)
PYSIDE_TEST(modelview_test.py)
PYSIDE_TEST(new_inherited_functions_test.py)
PYSIDE_TEST(notify_id.py)
PYSIDE_TEST(qvariant_test.py)
PYSIDE_TEST(signalandnamespace_test.py)
PYSIDE_TEST(signalwithdefaultvalue_test.py)
PYSIDE_TEST(signalemissionfrompython_test.py)
PYSIDE_TEST(version_test.py)
PYSIDE_TEST(typedef_signal_test.py)
PYSIDE_TEST(bug_1016.py)
PYSIDE_TEST(utils_test.py)
PYSIDE_TEST(mixin_signal_slots_test.py)
PYSIDE_TEST(signal_slot_warning.py)
