project(shibokengenerator)

set(shiboken_SRC
../generator.cpp
cppgenerator.cpp
headergenerator.cpp
overloaddata.cpp
shibokengenerator.cpp
main.cpp
)

include_directories(${generators_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${APIEXTRACTOR_INCLUDE_DIR}
                    ${Qt5Core_INCLUDE_DIRS}
                    )

add_executable(shiboken2 ${shiboken_SRC})
set_target_properties(shiboken2 PROPERTIES OUTPUT_NAME shiboken2${shiboken_SUFFIX})
target_link_libraries(shiboken2
                      ${APIEXTRACTOR_LIBRARY}
                      ${Qt5Core_LIBRARIES}
                      )

configure_file(shibokenconfig.h.in "${CMAKE_CURRENT_BINARY_DIR}/shibokenconfig.h" @ONLY)

install(TARGETS shiboken2 DESTINATION bin)
