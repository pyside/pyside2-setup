project(libshiboken)

option(ENABLE_VERSION_SUFFIX "Used to use current version in suffix to generated files. This is used to allow multiples versions installed simultaneous." FALSE)
if(ENABLE_VERSION_SUFFIX)
    set(shiboken_SUFFIX "-${shiboken_MAJOR_VERSION}.${shiboken_MINOR_VERSION}")
else()
    set(shiboken_SUFFIX "")
endif()

#Find installed sparsehash
find_path(SPARSEHASH_INCLUDE_PATH sparseconfig.h PATH_SUFFIXES "/google/sparsehash")
if(SPARSEHASH_INCLUDE_PATH)
    message(STATUS "Using system hash found in: ${SPARSEHASH_INCLUDE_PATH}")
else()
    set(SPARSEHASH_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/ext/sparsehash)
endif()

set(libshiboken_MAJOR_VERSION ${shiboken_MAJOR_VERSION})
set(libshiboken_MINOR_VERSION ${shiboken_MINOR_VERSION})
set(libshiboken_MICRO_VERSION ${shiboken_MICRO_VERSION})
set(libshiboken_VERSION "${libshiboken_MAJOR_VERSION}.${libshiboken_MINOR_VERSION}.${libshiboken_MICRO_VERSION}")
set(libshiboken_SOVERSION "${libshiboken_MAJOR_VERSION}.${libshiboken_MINOR_VERSION}")

set(libshiboken_SRC
basewrapper.cpp
gilstate.cpp
helper.cpp
sbkenum.cpp
bindingmanager.cpp
threadstatesaver.cpp
typeresolver.cpp
shibokenbuffer.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${SBK_PYTHON_INCLUDE_DIR}
                    ${SPARSEHASH_INCLUDE_PATH})
add_library(libshiboken SHARED ${libshiboken_SRC})
target_link_libraries(libshiboken ${SBK_PYTHON_LIBRARIES})
set_target_properties(libshiboken PROPERTIES OUTPUT_NAME "shiboken${shiboken_SUFFIX}-${PYTHON_BASENAME}"
                                             VERSION ${libshiboken_VERSION}
                                             SOVERSION ${libshiboken_SOVERSION}
                                             DEFINE_SYMBOL LIBSHIBOKEN_EXPORTS)

install(FILES
        autodecref.h
        basewrapper.h
        bindingmanager.h
        conversions.h
        gilstate.h
        helper.h
        sbkenum.h
        python25compat.h
        sbkdbg.h
        shiboken.h
        shibokenmacros.h
        threadstatesaver.h
        typeresolver.h
        shibokenbuffer.h
        DESTINATION include/shiboken${shiboken_SUFFIX})
install(TARGETS libshiboken EXPORT shiboken
                            LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
                            ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
                            RUNTIME DESTINATION bin)
