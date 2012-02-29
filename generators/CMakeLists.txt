project(shibokengenerator)

set(shiboken_SRC
generator.cpp
shiboken/cppgenerator.cpp
shiboken/headergenerator.cpp
shiboken/overloaddata.cpp
shiboken/shibokengenerator.cpp
shiboken/shibokennormalize.cpp
qtdoc/qtdocgenerator.cpp
main.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/shiboken
                    ${CMAKE_CURRENT_SOURCE_DIR}/qtdoc
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${APIEXTRACTOR_INCLUDE_DIR}
                    ${QT_INCLUDE_DIR}
                    ${QT_QTCORE_INCLUDE_DIR}
                    ${QT_QTXML_INCLUDE_DIR})

add_executable(shiboken ${shiboken_SRC})
set_target_properties(shiboken PROPERTIES OUTPUT_NAME shiboken${shiboken_SUFFIX})
target_link_libraries(shiboken
                      ${APIEXTRACTOR_LIBRARY}
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTXML_LIBRARY})

configure_file(shibokenconfig.h.in "${CMAKE_CURRENT_BINARY_DIR}/shibokenconfig.h" @ONLY)

install(TARGETS shiboken DESTINATION bin)
