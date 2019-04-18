add_executable(dumpcodemodel main.cpp)

target_include_directories(dumpcodemodel
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${apiextractor_SOURCE_DIR})

target_link_libraries(dumpcodemodel PUBLIC apiextractor Qt5::Core)
