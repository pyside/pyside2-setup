project(libminimal)

set(libminimal_SRC
obj.cpp
listuser.cpp
typedef.cpp
)

add_library(libminimal SHARED ${libminimal_SRC})
target_include_directories(libminimal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(libminimal PRIVATE LIBMINIMAL_BUILD)
set_property(TARGET libminimal PROPERTY PREFIX "")

