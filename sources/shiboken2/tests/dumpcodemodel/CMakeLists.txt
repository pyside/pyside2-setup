add_executable(dumpcodemodel main.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${apiextractor_SOURCE_DIR} ${Qt5Core_INCLUDE_DIRS})

target_link_libraries(dumpcodemodel apiextractor ${Qt5Core_LIBRARIES})
