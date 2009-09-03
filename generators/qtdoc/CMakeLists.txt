project(qtdoc_generator)

set(qtdoc_generator_SRC
qtdocgenerator.cpp
)

add_executable(docgenerator main.cpp)
target_link_libraries(docgenerator ${QT_QTCORE_LIBRARY})

add_library(qtdoc_generator SHARED ${qtdoc_generator_SRC})
target_link_libraries(qtdoc_generator ${APIEXTRACTOR_LIBRARY} ${QT_QTCORE_LIBRARY} genrunner)

install(TARGETS qtdoc_generator DESTINATION ${generator_plugin_DIR})
install(TARGETS docgenerator DESTINATION bin)

