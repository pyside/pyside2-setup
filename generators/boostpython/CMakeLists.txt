project(boostpython)

set(boostpython_SRC
boostpythongenerator.cpp
convertergenerator.cpp
cppgenerator.cpp
hppgenerator.cpp
boostpython.cpp
)

add_library(boostpython SHARED ${boostpython_SRC})
target_link_libraries(boostpython ${APIEXTRACTOR_LIBRARY} ${QT_QTCORE_LIBRARY} generator)

install(TARGETS boostpython DESTINATION lib)
