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

if (Qt5Positioning_VERSION VERSION_EQUAL 5.10.0 OR Qt5Positioning_VERSION VERSION_GREATER 5.10.0)
    list(APPEND QtPositioning_SRC
         ${QtPositioning_GEN_DIR}/qgeopolygon_wrapper.cpp)
endif()

set(QtPositioning_include_dirs ${QtPositioning_SOURCE_DIR}
                               ${QtPositioning_BINARY_DIR}
                               ${Qt5Core_INCLUDE_DIRS}
                               ${Qt5Positioning_INCLUDE_DIRS}
                               ${libpyside_SOURCE_DIR}
                               ${QtCore_GEN_DIR})

set(QtPositioning_libraries pyside2
                            ${Qt5Positioning_LIBRARIES})

set(QtPositioning_deps QtCore)

create_pyside_module(NAME QtPositioning
                     INCLUDE_DIRS QtPositioning_include_dirs
                     LIBRARIES QtPositioning_libraries
                     DEPS QtPositioning_deps
                     TYPESYSTEM_PATH QtPositioning_SOURCE_DIR
                     SOURCES QtPositioning_SRC
                     DROPPED_ENTRIES QtPositioning_DROPPED_ENTRIES)
