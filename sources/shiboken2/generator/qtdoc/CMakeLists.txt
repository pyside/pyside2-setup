project(qtdoc_generator)

set(qtdoc_generator_SRC
qtdocgenerator.cpp
)

include_directories(${generators_SOURCE_DIR}
                    ${Qt5Core_INCLUDE_DIRS}
                    ${APIEXTRACTOR_INCLUDE_DIR})
add_executable(docgenerator main.cpp)
set_target_properties(docgenerator PROPERTIES OUTPUT_NAME docgenerator${generator_SUFFIX})

target_link_libraries(docgenerator ${Qt5Core_LIBRARES})

add_library(qtdoc_generator SHARED ${qtdoc_generator_SRC})
target_link_libraries(qtdoc_generator ${APIEXTRACTOR_LIBRARY} ${Qt5Core_LIBRARES} genrunner)
set_property(TARGET qtdoc_generator PROPERTY PREFIX "")

install(TARGETS qtdoc_generator DESTINATION ${generator_plugin_DIR})
install(TARGETS docgenerator DESTINATION bin)

