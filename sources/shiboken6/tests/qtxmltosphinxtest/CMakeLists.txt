cmake_minimum_required(VERSION 3.16)

project(qtxmltosphinxtest)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Test)

set(generator_src_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../generator)
set(api_extractor_src_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../ApiExtractor)

set(qtxmltosphinxtest_SRC
    ${generator_src_dir}/qtdoc/qtxmltosphinx.cpp
    ${api_extractor_src_dir}/textstream.cpp
    qtxmltosphinxtest.cpp
    qtxmltosphinxtest.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
                    ${api_extractor_src_dir}
                    ${generator_src_dir}
                    ${generator_src_dir}/shiboken
                    ${generator_src_dir}/qtdoc)

add_executable(qtxmltosphinxtest ${qtxmltosphinxtest_SRC})

target_link_libraries(qtxmltosphinxtest PRIVATE
                      Qt::Core
                      Qt::Test)

add_test("qtxmltosphinx" qtxmltosphinxtest)
if (INSTALL_TESTS)
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/qtxmltosphinxtest DESTINATION ${TEST_INSTALL_DIR})
endif()
