project(pysidetest)
project(testbinding)

cmake_minimum_required(VERSION 3.16)

set(QT_USE_QTCORE 1)
# no more supported: include(${QT_USE_FILE})

# removed after qtbase/054b66a65748c9ebfafeca88bf31669a24994237, is this required?
# add_definitions(${Qt${QT_MAJOR_VERSION}Core_DEFINITIONS})

add_definitions(-DQT_SHARED)
add_definitions(-DRXX_ALLOCATOR_INIT_0)

find_package(Qt${QT_MAJOR_VERSION}Widgets)

set(pysidetest_SRC
flagstest.cpp
testobject.cpp
testview.cpp
hiddenobject.cpp
)

set(pysidetest_MOC_HEADERS
flagstest.h
testobject.h
testview.h
hiddenobject.h
)

qt_wrap_cpp(pysidetest_MOC_SRC ${pysidetest_MOC_HEADERS})

set(testbinding_SRC
${CMAKE_CURRENT_BINARY_DIR}/testbinding/flagsnamespace_classforenum_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/testbinding/testobject_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/testbinding/intvalue_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/testbinding/pysidecpp_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/testbinding/pysidecpp_testobjectwithnamespace_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/testbinding/pysidecpp_testobject2withnamespace_wrapper.cpp
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

if(${QT_MAJOR_VERSION} GREATER_EQUAL 6)
    set(Qt6Core_DIRECT_INCLUDE_DIR ${Qt6Core_INCLUDE_DIRS})
    set(Qt6Gui_DIRECT_INCLUDE_DIR ${Qt6Gui_INCLUDE_DIRS})
    set(Qt6Widgets_DIRECT_INCLUDE_DIR ${Qt6Widgets_INCLUDE_DIRS})
else()
    list(GET Qt5Core_INCLUDE_DIRS 1 Qt5Core_DIRECT_INCLUDE_DIR)
    list(GET Qt5Gui_INCLUDE_DIRS 1 Qt5Gui_DIRECT_INCLUDE_DIR)
    list(GET Qt5Widgets_INCLUDE_DIRS 1 Qt5Widgets_DIRECT_INCLUDE_DIR)
endif()

# Adjust include headers paths for frameworks.
set(shiboken_framework_include_dirs_option "")
if(CMAKE_HOST_APPLE AND QtCore_is_framework)
    set(shiboken_framework_include_dirs "${QT_FRAMEWORK_INCLUDE_DIR}")
    set(shiboken_framework_include_dirs_option "--framework-include-paths=${shiboken_framework_include_dirs}")
endif()

make_path(testbinding_include_dirs  ${pyside2_BINARY_DIR}
                                    ${CMAKE_CURRENT_SOURCE_DIR}
                                    ${CMAKE_CURRENT_SOURCE_DIR}/../../../shiboken2/libshiboken
                                    ${CMAKE_CURRENT_SOURCE_DIR}/../../PySide2
                                    ${CMAKE_CURRENT_SOURCE_DIR}/../../libpyside
                                    ${QT_INCLUDE_DIR}
                                    ${Qt${QT_MAJOR_VERSION}Core_DIRECT_INCLUDE_DIR}
                                    ${Qt${QT_MAJOR_VERSION}Gui_DIRECT_INCLUDE_DIR}
                                    ${Qt${QT_MAJOR_VERSION}Widgets_DIRECT_INCLUDE_DIR}
                                    )

make_path(testbinding_typesystem_path ${pyside2_SOURCE_DIR}
                                      ${pyside2_BINARY_DIR})

add_custom_command(
OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/mjb_rejected_classes.log"
        BYPRODUCTS ${testbinding_SRC}
        COMMAND Shiboken2::shiboken2 ${GENERATOR_EXTRA_FLAGS}
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
                    ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                    ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                    ${Qt${QT_MAJOR_VERSION}Widgets_INCLUDE_DIRS}
                    ${pyside2_SOURCE_DIR}
                    ${QtCore_GEN_DIR}
                    ${QtGui_GEN_DIR}
                    ${QtWidgets_GEN_DIR}
                    ${libpyside_SOURCE_DIR})

add_library(pysidetest SHARED ${pysidetest_SRC} ${pysidetest_MOC_SRC})
set_target_properties(pysidetest PROPERTIES
                      DEFINE_SYMBOL BUILD_PYSIDETEST)

target_link_libraries(pysidetest
                      Shiboken2::libshiboken
                      ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                      ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES}
                      ${Qt${QT_MAJOR_VERSION}Widgets_LIBRARIES}
                      )

add_library(testbinding MODULE ${testbinding_SRC})
set_property(TARGET testbinding PROPERTY PREFIX "")
set_property(TARGET testbinding PROPERTY OUTPUT_NAME "testbinding${SHIBOKEN_PYTHON_EXTENSION_SUFFIX}")
if(WIN32)
    set_property(TARGET testbinding PROPERTY SUFFIX ".pyd")
endif()

if(PYTHON_LIMITED_API)
    set(TESTBINDING_PYTHON_LIBS ${PYTHON_LIMITED_LIBRARIES})
else()
    set(TESTBINDING_PYTHON_LIBS ${PYTHON_LIBRARIES})
endif()

target_link_libraries(testbinding
                      pysidetest
                      pyside2
                      ${TESTBINDING_PYTHON_LIBS}
                      Shiboken2::libshiboken
                      ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                      ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES}
                      ${Qt${QT_MAJOR_VERSION}Widgets_LIBRARIES})

add_dependencies(testbinding pyside2 QtCore QtGui QtWidgets pysidetest)
create_generator_target(testbinding)

PYSIDE_TEST(constructor_properties_test.py)
PYSIDE_TEST(decoratedslot_test.py)
# Will always crash when built against Qt 5.6, no point in running it.
if (Qt${QT_MAJOR_VERSION}Core_VERSION VERSION_GREATER 5.7.0)
    PYSIDE_TEST(delegatecreateseditor_test.py)
endif()
PYSIDE_TEST(all_modules_load_test.py)
PYSIDE_TEST(bug_1016.py)
PYSIDE_TEST(embedding_test.py)
PYSIDE_TEST(enum_test.py)
PYSIDE_TEST(homonymoussignalandmethod_test.py)
PYSIDE_TEST(iterable_test.py)
PYSIDE_TEST(list_signal_test.py)
PYSIDE_TEST(mixin_signal_slots_test.py)
PYSIDE_TEST(modelview_test.py)
PYSIDE_TEST(new_inherited_functions_test.py)
PYSIDE_TEST(notify_id.py)
PYSIDE_TEST(properties_test.py)
PYSIDE_TEST(qapp_like_a_macro_test.py)
PYSIDE_TEST(qvariant_test.py)
PYSIDE_TEST(repr_test.py)
PYSIDE_TEST(signal_tp_descr_get_test.py)
PYSIDE_TEST(signal_slot_warning.py)
PYSIDE_TEST(signalandnamespace_test.py)
PYSIDE_TEST(signalemissionfrompython_test.py)
PYSIDE_TEST(signalwithdefaultvalue_test.py)
PYSIDE_TEST(typedef_signal_test.py)
PYSIDE_TEST(version_test.py)
