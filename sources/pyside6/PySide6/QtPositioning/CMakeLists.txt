project(QtPositioning)

set(QtPositioning_OPTIONAL_SRC )
set(QtPositioning_DROPPED_ENTRIES )

set(QtPositioning_SRC
${QtPositioning_GEN_DIR}/qgeoaddress_wrapper.cpp
${QtPositioning_GEN_DIR}/qgeoareamonitorinfo_wrapper.cpp
${QtPositioning_GEN_DIR}/qgeoareamonitorsource_wrapper.cpp
${QtPositioning_GEN_DIR}/qgeolocation_wrapper.cpp
${QtPositioning_GEN_DIR}/qgeocircle_wrapper.cpp
${QtPositioning_GEN_DIR}/qgeocoordinate_wrapper.cpp
${QtPositioning_GEN_DIR}/qgeopath_wrapper.cpp
${QtPositioning_GEN_DIR}/qgeopositioninfo_wrapper.cpp
${QtPositioning_GEN_DIR}/qgeopositioninfosource_wrapper.cpp
${QtPositioning_GEN_DIR}/qgeopositioninfosourcefactory_wrapper.cpp
${QtPositioning_GEN_DIR}/qgeorectangle_wrapper.cpp
${QtPositioning_GEN_DIR}/qgeosatelliteinfo_wrapper.cpp
${QtPositioning_GEN_DIR}/qgeosatelliteinfosource_wrapper.cpp
${QtPositioning_GEN_DIR}/qgeoshape_wrapper.cpp
${QtPositioning_GEN_DIR}/qnmeapositioninfosource_wrapper.cpp
# module is always needed
${QtPositioning_GEN_DIR}/qtpositioning_module_wrapper.cpp
)

if (Qt${QT_MAJOR_VERSION}Positioning_VERSION VERSION_EQUAL 5.10.0
    OR Qt${QT_MAJOR_VERSION}Positioning_VERSION VERSION_GREATER 5.10.0)
    list(APPEND QtPositioning_SRC
         ${QtPositioning_GEN_DIR}/qgeopolygon_wrapper.cpp)
endif()

set(QtPositioning_include_dirs ${QtPositioning_SOURCE_DIR}
                               ${QtPositioning_BINARY_DIR}
                               ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                               ${Qt${QT_MAJOR_VERSION}Positioning_INCLUDE_DIRS}
                               ${libpyside_SOURCE_DIR}
                               ${QtCore_GEN_DIR})

set(QtPositioning_libraries pyside6
                            ${Qt${QT_MAJOR_VERSION}Positioning_LIBRARIES})

set(QtPositioning_deps QtCore)

create_pyside_module(NAME QtPositioning
                     INCLUDE_DIRS QtPositioning_include_dirs
                     LIBRARIES QtPositioning_libraries
                     DEPS QtPositioning_deps
                     TYPESYSTEM_PATH QtPositioning_SOURCE_DIR
                     SOURCES QtPositioning_SRC
                     DROPPED_ENTRIES QtPositioning_DROPPED_ENTRIES)
