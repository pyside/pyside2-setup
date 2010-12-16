project(libsample)

set(libsample_SRC
abstract.cpp
blackbox.cpp
bucket.cpp
collector.cpp
complex.cpp
derived.cpp
echo.cpp
functions.cpp
implicitconv.cpp
injectcode.cpp
listuser.cpp
modifications.cpp
mapuser.cpp
modified_constructor.cpp
multiple_derived.cpp
objectmodel.cpp
objecttype.cpp
objecttypelayout.cpp
objectview.cpp
overload.cpp
overloadsort.cpp
pairuser.cpp
point.cpp
pointf.cpp
polygon.cpp
protected.cpp
reference.cpp
samplenamespace.cpp
simplefile.cpp
size.cpp
sometime.cpp
str.cpp
strlist.cpp
virtualmethods.cpp
expression.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_definitions("-DLIBSAMPLE_BUILD")
add_library(libsample SHARED ${libsample_SRC})
set_property(TARGET libsample PROPERTY PREFIX "")

