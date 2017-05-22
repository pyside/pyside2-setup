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
# QtDeclarative is no longer supported
