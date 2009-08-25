project(boostpython)

set(boostpython_generator_SRC
boostpythongenerator.cpp
convertergenerator.cpp
cppgenerator.cpp
hppgenerator.cpp
boostpython.cpp
)

add_library(boostpython_generator SHARED ${boostpython_generator_SRC})
target_link_libraries(boostpython_generator ${APIEXTRACTOR_LIBRARY} ${QT_QTCORE_LIBRARY} genrunner)

install(TARGETS boostpython_generator DESTINATION ${LIB_INSTALL_DIR})
