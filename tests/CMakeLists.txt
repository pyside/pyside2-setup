
qt4_automoc(testabstractmetaclass.cpp)
add_executable(testabstractmetaclass testabstractmetaclass.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${apiextractor_SOURCE_DIR})
target_link_libraries(testabstractmetaclass ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} apiextractor)
add_test("AbstractMetaClass" testabstractmetaclass)
