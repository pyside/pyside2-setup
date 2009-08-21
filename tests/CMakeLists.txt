project(sphinxtabletest)

# TODO
set(sphinxtabletest_SRC sphinxtabletest.cpp)
qt4_automoc(${sphinxtabletest_SRC})
include_directories(${QT_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${qtdoc_generator_SOURCE_DIR})
add_executable(sphinxtabletest ${sphinxtabletest_SRC})
target_link_libraries(sphinxtabletest ${QT_QTTEST_LIBRARY} ${APIEXTRACTOR_LIBRARY} qtdoc_generator generator)
add_test("sphinxtable" sphinxtabletest)
