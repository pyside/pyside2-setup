project(libsmart)

set(libsmart_SRC
smart.cpp
)

add_library(libsmart SHARED ${libsmart_SRC})
target_include_directories(libsmart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(libsmart PRIVATE LIBSMART_BUILD)
set_property(TARGET libsmart PROPERTY PREFIX "")

