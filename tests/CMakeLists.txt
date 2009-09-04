
macro(declare_test testname)
    qt4_automoc("${testname}.cpp")
    add_executable(${testname} "${testname}.cpp")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${apiextractor_SOURCE_DIR})
    target_link_libraries(${testname} ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} apiextractor)
    add_test(${testname} ${testname})
endmacro(declare_test testname)

declare_test(testabstractmetaclass)
declare_test(testabstractmetatype)
declare_test(testenum)
