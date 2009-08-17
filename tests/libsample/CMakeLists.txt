project(libsample)

set(libsample_SRC
abstract.cpp
complex.cpp
derived.cpp
functions.cpp
implicitconv.cpp
kindergarten.cpp
listuser.cpp
modifications.cpp
pairuser.cpp
point.cpp
reference.cpp
samplenamespace.cpp
size.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(libsample SHARED ${libsample_SRC})
set_property(TARGET libsample PROPERTY PREFIX "")

