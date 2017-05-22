project(shibokengenerator)

find_package(Qt5Xml)
find_package(Qt5XmlPatterns)

set(shiboken2_SRC
generator.cpp
shiboken2/cppgenerator.cpp
shiboken2/headergenerator.cpp
shiboken2/overloaddata.cpp
shiboken2/shibokengenerator.cpp
main.cpp
)

if (NOT DISABLE_DOCSTRINGS)
    set(shiboken2_SRC ${shiboken2_SRC} qtdoc/qtdocgenerator.cpp)
    add_definitions(-DDOCSTRINGS_ENABLED)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/shiboken2
                    ${CMAKE_CURRENT_SOURCE_DIR}/qtdoc
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${apiextractor_SOURCE_DIR}
                    ${Qt5Core_INCLUDE_DIRS}
                    ${Qt5XmlPatterns_INCLUDE_DIRS}
                    )

add_executable(shiboken2 ${shiboken2_SRC})
add_dependencies(shiboken2 apiextractor)
set_target_properties(shiboken2 PROPERTIES OUTPUT_NAME shiboken2${shiboken2_SUFFIX})
target_link_libraries(shiboken2
                      apiextractor
                      ${Qt5Core_LIBRARIES}
                      ${Qt5XmlPatterns_LIBRARIES}
                      )

configure_file(shibokenconfig.h.in "${CMAKE_CURRENT_BINARY_DIR}/shibokenconfig.h" @ONLY)

install(TARGETS shiboken2 DESTINATION bin)
