project(generators)

set(shiboken_SRC
cppgenerator.cpp
headergenerator.cpp
overloaddata.cpp
shiboken.cpp
shibokengenerator.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${APIEXTRACTOR_INCLUDE_DIR}
                    ${GENERATORRUNNER_INCLUDE_DIR}
                    ${QT_INCLUDE_DIR}
                    ${QT_QTCORE_INCLUDE_DIR})

add_library(shiboken_generator SHARED ${shiboken_SRC})
set_property(TARGET shiboken_generator PROPERTY PREFIX "")

target_link_libraries(shiboken_generator
                      ${APIEXTRACTOR_LIBRARY}
                      ${GENERATORRUNNER_LIBRARY}
                      ${QT_QTCORE_LIBRARY})

add_executable(shiboken main.cpp)
set_target_properties(shiboken PROPERTIES OUTPUT_NAME shiboken${shiboken_SUFFIX})
target_link_libraries(shiboken ${QT_QTCORE_LIBRARY})

add_dependencies(shiboken shiboken_generator)

install(TARGETS shiboken_generator DESTINATION "${GENERATORRUNNER_PLUGIN_DIR}")
install(TARGETS shiboken DESTINATION bin)
