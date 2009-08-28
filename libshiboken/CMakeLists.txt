project(libshiboken)

find_package(PythonLibs REQUIRED)

set(libshiboken_VERSION 0.1)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/FindShiboken.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/FindShiboken.cmake @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/shiboken.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/shiboken.pc @ONLY)

set(libshiboken_SRC
basewrapper.cpp
containers.cpp
helper.cpp
pyenum.cpp
bindingmanager.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${PYTHON_INCLUDE_PATH})
add_library(libshiboken SHARED ${libshiboken_SRC})
set_property(TARGET libshiboken PROPERTY PREFIX "")
target_link_libraries(libshiboken
                      ${PYTHON_LIBRARIES}
                      -lutil)

install(DIRECTORY . DESTINATION include/shiboken
        FILES_MATCHING PATTERN "*.h"
        PATTERN ".git" EXCLUDE
        )
install(TARGETS libshiboken DESTINATION ${LIB_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/FindShiboken.cmake
        DESTINATION share/cmake-2.6/Modules)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/shiboken.pc
        DESTINATION "${LIB_INSTALL_DIR}/pkgconfig")

