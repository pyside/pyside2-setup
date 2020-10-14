project(libsample)

set(libsample_SRC
abstract.cpp
blackbox.cpp
bytearray.cpp
bucket.cpp
collector.cpp
complex.cpp
onlycopy.cpp
derived.cpp
echo.cpp
exceptiontest.cpp
functions.cpp
handle.cpp
implicitconv.cpp
injectcode.cpp
listuser.cpp
modifications.cpp
mapuser.cpp
modified_constructor.cpp
multiple_derived.cpp
oddbool.cpp
objectmodel.cpp
objecttype.cpp
objecttypeholder.cpp
objecttypelayout.cpp
objecttypeoperators.cpp
objectview.cpp
overload.cpp
overloadsort.cpp
pairuser.cpp
pen.cpp
photon.cpp
point.cpp
pointf.cpp
polygon.cpp
protected.cpp
reference.cpp
renaming.cpp
sample.cpp
samplenamespace.cpp
sbkdate.cpp
simplefile.cpp
size.cpp
sometime.cpp
str.cpp
strlist.cpp
templateptr.cpp
transform.cpp
typesystypedef.cpp
virtualmethods.cpp
expression.cpp
filter.cpp
)

add_library(libsample SHARED ${libsample_SRC})
target_include_directories(libsample PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(libsample PRIVATE LIBSAMPLE_BUILD)
set_property(TARGET libsample PROPERTY PREFIX "")

