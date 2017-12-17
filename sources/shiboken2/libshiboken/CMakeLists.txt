project(libshiboken)

macro(get_numpy_location)
    execute_process(
      COMMAND ${PYTHON_EXECUTABLE} -c "if True:
      import sys
      import os
      numpy = ''
      for p in sys.path:
          if 'site-' in p:
              numpy = os.path.join(p, 'numpy')
              if os.path.exists(numpy):
                  print(os.path.realpath(numpy))
                  break"
      OUTPUT_VARIABLE PYTHON_NUMPY_LOCATION
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  message("PYTHON_NUMPY_LOCATION: " ${PYTHON_NUMPY_LOCATION})
endmacro()

option(ENABLE_VERSION_SUFFIX "Used to use current version in suffix to generated files. This is used to allow multiples versions installed simultaneous." FALSE)
if(ENABLE_VERSION_SUFFIX)
    set(shiboken2_SUFFIX "-${shiboken_MAJOR_VERSION}.${shiboken_MINOR_VERSION}")
else()
    set(shiboken2_SUFFIX "")
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/sbkversion.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/sbkversion.h" @ONLY)

set(libshiboken_MAJOR_VERSION ${shiboken_MAJOR_VERSION})
set(libshiboken_MINOR_VERSION ${shiboken_MINOR_VERSION})
set(libshiboken_MICRO_VERSION ${shiboken_MICRO_VERSION})
set(libshiboken_VERSION "${libshiboken_MAJOR_VERSION}.${libshiboken_MINOR_VERSION}.${libshiboken_MICRO_VERSION}")
set(libshiboken_SOVERSION "${shiboken2_library_so_version}")

set(libshiboken_SRC
basewrapper.cpp
debugfreehook.cpp
gilstate.cpp
helper.cpp
sbkarrayconverter.cpp
sbkconverter.cpp
sbkenum.cpp
sbkmodule.cpp
sbkstring.cpp
bindingmanager.cpp
threadstatesaver.cpp
shibokenbuffer.cpp
signature.cpp
qapp_macro.cpp
pep384resolve.cpp
voidptr.cpp
)

get_numpy_location()

set(libshiboken_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${SBK_PYTHON_INCLUDE_DIR})

if (NOT "${PYTHON_NUMPY_LOCATION}" STREQUAL "")
    set(libshiboken_INCLUDES ${libshiboken_INCLUDES} ${PYTHON_NUMPY_LOCATION}/core/include)
    set(libshiboken_SRC ${libshiboken_SRC} sbknumpyarrayconverter.cpp)
    add_definitions(-DHAVE_NUMPY -DNPY_NO_DEPRECATED_API=NPY_1_7_API_VERSION)
endif()

set(APIEXTRACTOR_EXTRA_INCLUDES ${APIEXTRACTOR_EXTRA_INCLUDES} ${LIBXSLT_INCLUDE_DIR} ${LIBXML2_INCLUDE_DIR})

include_directories(${libshiboken_INCLUDES})
add_library(libshiboken SHARED ${libshiboken_SRC})
target_link_libraries(libshiboken ${SBK_PYTHON_LIBRARIES})
set_target_properties(libshiboken PROPERTIES OUTPUT_NAME "shiboken2${shiboken2_SUFFIX}${PYTHON_SHARED_LIBRARY_SUFFIX}"
                                             VERSION ${libshiboken_VERSION}
                                             SOVERSION ${libshiboken_SOVERSION}
                                             DEFINE_SYMBOL LIBSHIBOKEN_EXPORTS)

install(FILES
        autodecref.h
        basewrapper.h
        bindingmanager.h
        gilstate.h
        helper.h
        sbkarrayconverter.h
        sbkconverter.h
        sbkenum.h
        sbkmodule.h
        python25compat.h
        sbkdbg.h
        sbkstring.h
        shiboken.h
        shibokenmacros.h
        threadstatesaver.h
        shibokenbuffer.h
        sbkpython.h
        pep384resolve.h
        signature.h
        qapp_macro.h
        voidptr.h
        "${CMAKE_CURRENT_BINARY_DIR}/sbkversion.h"
        DESTINATION include/shiboken2${shiboken2_SUFFIX})
install(TARGETS libshiboken EXPORT shiboken2
                            LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
                            ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
                            RUNTIME DESTINATION bin)
