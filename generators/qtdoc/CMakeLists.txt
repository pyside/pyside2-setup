project(qtdoc_generator)

set(qtdoc_generator_SRC
qtdocgenerator.cpp
)

add_library(qtdoc_generator SHARED ${qtdoc_generator_SRC})
target_link_libraries(qtdoc_generator ${APIEXTRACTOR_LIBRARY} ${QT_QTCORE_LIBRARY} genrunner)

install(TARGETS qtdoc_generator DESTINATION ${LIB_INSTALL_DIR})
