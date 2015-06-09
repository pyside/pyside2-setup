#
# Try to find QtMultimedia
# TODO: Remove this hack when cmake support QtMultimedia module
# CT: maybe we can remove this.
# For now, I just use the mapping to Qt5

find_package(Qt5Multimedia)

if (NOT Qt5Multimedia_FOUND)
    find_path(QT_QTMULTIMEDIA_INCLUDE_DIR QtMultimedia
            PATHS ${QT_HEADERS_DIR}/QtMultimedia
                ${QT_LIBRARY_DIR}/QtMultimedia.framework/Headers
            NO_DEFAULT_PATH)
    find_library(QT_QTMULTIMEDIA_LIBRARY QtMultimedia PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH)
    if (QT_QTMULTIMEDIA_INCLUDE_DIR AND QT_QTMULTIMEDIA_LIBRARY)
        set(QT_QTMULTIMEDIA_FOUND ON)
    else()
        #Replace this on documentation
        set(if_QtMultimedia "<!--")
        set(end_QtMultimedia "-->")
    endif()
endif ()

# Maemo is no longer supported

# Try to find QtDeclarative
# TODO: Remove this hack when cmake support QtDeclarative module
find_package(Qt5Declarative)
if (NOT Qt5Declarative_FOUND)
    find_path(QT_QTDECLARATIVE_INCLUDE_DIR QtDeclarative
            PATHS ${QT_HEADERS_DIR}/QtDeclarative
                ${QT_LIBRARY_DIR}/QtDeclarative.framework/Headers
            NO_DEFAULT_PATH)
    find_library(QT_QTDECLARATIVE_LIBRARY QtDeclarative PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH)
    if (QT_QTDECLARATIVE_INCLUDE_DIR AND QT_QTDECLARATIVE_LIBRARY)
        set(QT_QTDECLARATIVE_FOUND ON)
    else()
        #Replace this on documentation
        set(if_QtDeclarative "<!--")
        set(end_QtDeclarative "-->")
    endif()
endif ()


