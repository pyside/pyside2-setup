project(sphinxtabletest)

# TODO
set(sphinxtabletest_SRC sphinxtabletest.cpp)
qt4_automoc(${sphinxtabletest_SRC})
include_directories(${QT_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${boostpythongenerator_SOURCE_DIR})
add_executable(sphinxtabletest ${sphinxtabletest_SRC})
target_link_libraries(sphinxtabletest ${QT_QTTEST_LIBRARY} ${APIEXTRACTOR_LIBRARY} libboostpythongenerator)
add_test("sphinxtable" sphinxtabletest)
