project(libsmart)

set(libsmart_SRC
smart.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_definitions("-DLIBSMART_BUILD")
add_library(libsmart SHARED ${libsmart_SRC})
set_property(TARGET libsmart PROPERTY PREFIX "")

