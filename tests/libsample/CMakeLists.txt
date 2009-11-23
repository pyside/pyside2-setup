project(libsample)

set(libsample_SRC
abstract.cpp
collector.cpp
complex.cpp
derived.cpp
echo.cpp
functions.cpp
implicitconv.cpp
injectcode.cpp
kindergarten.cpp
listuser.cpp
modifications.cpp
mapuser.cpp
multiple_derived.cpp
objecttype.cpp
overload.cpp
pairuser.cpp
point.cpp
reference.cpp
samplenamespace.cpp
simplefile.cpp
size.cpp
sometime.cpp
str.cpp
virtualmethods.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_definitions("-DLIBSAMPLE_BUILD")
add_library(libsample SHARED ${libsample_SRC})
set_property(TARGET libsample PROPERTY PREFIX "")

