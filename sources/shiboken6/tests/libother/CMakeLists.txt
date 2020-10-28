project(libother)

set(libother_SRC
number.cpp
otherderived.cpp
otherobjecttype.cpp
othermultiplederived.cpp
smartptrtester.cpp
othertypesystypedef.cpp
)

add_library(libother SHARED ${libother_SRC})
target_include_directories(libother PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(libother PRIVATE LIBOTHER_BUILD)
target_link_libraries(libother PUBLIC libsample libsmart)
set_property(TARGET libother PROPERTY PREFIX "")


