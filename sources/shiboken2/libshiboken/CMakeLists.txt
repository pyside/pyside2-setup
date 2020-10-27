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
  message(STATUS "PYTHON_NUMPY_LOCATION: " ${PYTHON_NUMPY_LOCATION})
endmacro()

option(ENABLE_VERSION_SUFFIX "Used to use current version in suffix to generated files. This is used to allow multiples versions installed simultaneous." FALSE)
if(ENABLE_VERSION_SUFFIX)
    set(shiboken2_SUFFIX "-${shiboken_MAJOR_VERSION}.${shiboken_MINOR_VERSION}")
else()
    set(shiboken2_SUFFIX "")
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/sbkversion.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/sbkversion.h" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/embed/signature_bootstrap.py"
               "${CMAKE_CURRENT_BINARY_DIR}/embed/signature_bootstrap.py" @ONLY)

add_custom_command(
    OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/embed/signature_bootstrap_inc.h"
    OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/embed/signature_inc.h"
    COMMAND ${PYTHON_EXECUTABLE} -E
            "${CMAKE_CURRENT_SOURCE_DIR}/embed/embedding_generator.py"
            --cmake-dir "${CMAKE_CURRENT_BINARY_DIR}/embed"
            --limited-api ${PYTHON_LIMITED_API})

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
sbkstaticstrings.cpp
bindingmanager.cpp
threadstatesaver.cpp
shibokenbuffer.cpp
qapp_macro.cpp
pep384impl.cpp
voidptr.cpp
bufferprocs_py37.cpp

embed/signature_bootstrap_inc.h
embed/signature_inc.h

signature/signature.cpp
signature/signature_globals.cpp
signature/signature_extend.cpp
signature/signature_helper.cpp
)

get_numpy_location()

if (NOT "${PYTHON_NUMPY_LOCATION}" STREQUAL "")
    list(APPEND libshiboken_SRC sbknumpyarrayconverter.cpp)
endif()

set(APIEXTRACTOR_EXTRA_INCLUDES ${APIEXTRACTOR_EXTRA_INCLUDES} ${LIBXSLT_INCLUDE_DIR} ${LIBXML2_INCLUDE_DIR})

add_library(libshiboken SHARED ${libshiboken_SRC})
add_library(Shiboken2::libshiboken ALIAS libshiboken)

target_include_directories(libshiboken PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:include/shiboken2>
)

if (NOT "${PYTHON_NUMPY_LOCATION}" STREQUAL "")
    target_include_directories(libshiboken PRIVATE ${PYTHON_NUMPY_LOCATION}/core/include)
    target_compile_definitions(libshiboken PRIVATE -DHAVE_NUMPY
                                           PRIVATE -DNPY_NO_DEPRECATED_API=NPY_1_7_API_VERSION)

endif()

shiboken_compute_python_includes()
# On Windows we need to link against the python.lib import library.
# On macOS and Linux we don't link against the python shared / static library,
# the dynamic linker will pick up the python symbols at runtime automatically.
shiboken_compute_python_libraries()

if(PYTHON_LIMITED_API)
    target_compile_definitions(libshiboken PUBLIC "-DPy_LIMITED_API=0x03050000")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(PYTHON_WITH_DEBUG)
        target_compile_definitions(libshiboken PUBLIC "-DPy_DEBUG")
    endif()
    if (PYTHON_WITH_COUNT_ALLOCS)
        target_compile_definitions(libshiboken PUBLIC "-DCOUNT_ALLOCS")
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(libshiboken PUBLIC "-DNDEBUG")
endif()

set_target_properties(libshiboken PROPERTIES OUTPUT_NAME "shiboken2${shiboken2_SUFFIX}${PYTHON_SHARED_LIBRARY_SUFFIX}"
                                             VERSION ${libshiboken_VERSION}
                                             SOVERSION ${libshiboken_SOVERSION}
                                             DEFINE_SYMBOL BUILD_LIBSHIBOKEN)

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
        sbkdbg.h
        sbkstring.h
        sbkstaticstrings.h
        shiboken.h
        shibokenmacros.h
        threadstatesaver.h
        shibokenbuffer.h
        sbkpython.h
        pep384impl.h
        qapp_macro.h
        voidptr.h
        bufferprocs_py37.h
        "${CMAKE_CURRENT_BINARY_DIR}/sbkversion.h"

        signature.h
        signature/signature_p.h

        DESTINATION include/shiboken2${shiboken2_SUFFIX})
install(TARGETS libshiboken EXPORT Shiboken2Targets
                            LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
                            ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
                            RUNTIME DESTINATION bin)
install(EXPORT Shiboken2Targets NAMESPACE Shiboken2::
        DESTINATION ${LIB_INSTALL_DIR}/cmake/Shiboken2-${shiboken2_VERSION})
