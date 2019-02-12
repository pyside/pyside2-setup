project(QtSensors)

set(QtSensors_OPTIONAL_SRC )
set(QtSensors_DROPPED_ENTRIES )

set(QtSensors_SRC
#   overrides QObject::metaObject() by private method
#   ${QtSensors_GEN_DIR}/qsensorgesture_wrapper.cpp
    ${QtSensors_GEN_DIR}/qsensorgesturemanager_wrapper.cpp
    ${QtSensors_GEN_DIR}/qsensorgestureplugininterface_wrapper.cpp
    ${QtSensors_GEN_DIR}/qsensorgesturerecognizer_wrapper.cpp
    ${QtSensors_GEN_DIR}/qaccelerometer_wrapper.cpp
    ${QtSensors_GEN_DIR}/qaccelerometerfilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qaccelerometerreading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qaltimeter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qaltimeterfilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qaltimeterreading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qambientlightfilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qambientlightreading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qambientlightsensor_wrapper.cpp
    ${QtSensors_GEN_DIR}/qambienttemperaturefilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qambienttemperaturereading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qambienttemperaturesensor_wrapper.cpp
    ${QtSensors_GEN_DIR}/qcompass_wrapper.cpp
    ${QtSensors_GEN_DIR}/qcompassfilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qcompassreading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qdistancefilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qdistancereading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qdistancesensor_wrapper.cpp
    ${QtSensors_GEN_DIR}/qgyroscope_wrapper.cpp
    ${QtSensors_GEN_DIR}/qgyroscopefilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qgyroscopereading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qholsterfilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qholsterreading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qholstersensor_wrapper.cpp
    ${QtSensors_GEN_DIR}/qhumidityfilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qhumidityreading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qhumiditysensor_wrapper.cpp
    ${QtSensors_GEN_DIR}/qirproximityfilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qirproximityreading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qirproximitysensor_wrapper.cpp
    ${QtSensors_GEN_DIR}/qlidfilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qlidreading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qlidsensor_wrapper.cpp
    ${QtSensors_GEN_DIR}/qlightfilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qlightreading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qlightsensor_wrapper.cpp
    ${QtSensors_GEN_DIR}/qmagnetometer_wrapper.cpp
    ${QtSensors_GEN_DIR}/qmagnetometerfilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qmagnetometerreading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qorientationfilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qorientationreading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qorientationsensor_wrapper.cpp
    ${QtSensors_GEN_DIR}/qpressurefilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qpressurereading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qpressuresensor_wrapper.cpp
    ${QtSensors_GEN_DIR}/qproximityfilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qproximityreading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qproximitysensor_wrapper.cpp
    ${QtSensors_GEN_DIR}/qrotationfilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qrotationreading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qrotationsensor_wrapper.cpp
    ${QtSensors_GEN_DIR}/qsensor_wrapper.cpp
    ${QtSensors_GEN_DIR}/qsensorfilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qsensorreading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qoutputrange_wrapper.cpp
    ${QtSensors_GEN_DIR}/qsensorbackend_wrapper.cpp
    ${QtSensors_GEN_DIR}/qsensorbackendfactory_wrapper.cpp
    ${QtSensors_GEN_DIR}/qsensormanager_wrapper.cpp
    ${QtSensors_GEN_DIR}/qsensorchangesinterface_wrapper.cpp
    ${QtSensors_GEN_DIR}/qsensorplugininterface_wrapper.cpp
    ${QtSensors_GEN_DIR}/qtapfilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qtapreading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qtapsensor_wrapper.cpp
    ${QtSensors_GEN_DIR}/qtiltfilter_wrapper.cpp
    ${QtSensors_GEN_DIR}/qtiltreading_wrapper.cpp
    ${QtSensors_GEN_DIR}/qtiltsensor_wrapper.cpp
# module is always needed
    ${QtSensors_GEN_DIR}/qtsensors_module_wrapper.cpp
)

set(QtSensors_include_dirs  ${QtSensors_SOURCE_DIR}
                            ${QtSensors_BINARY_DIR}
                            ${Qt5Core_INCLUDE_DIRS}
                            ${Qt5Sensors_INCLUDE_DIRS}
                            ${libpyside_SOURCE_DIR}
                            ${QtCore_GEN_DIR})

set(QtSensors_libraries pyside2
                        ${Qt5Sensors_LIBRARIES})

set(QtSensors_deps QtCore)

create_pyside_module(NAME QtSensors
                     INCLUDE_DIRS QtSensors_include_dirs
                     LIBRARIES QtSensors_libraries
                     DEPS QtSensors_deps
                     TYPESYSTEM_PATH QtSensors_SOURCE_DIR
                     SOURCES QtSensors_SRC
                     DROPPED_ENTRIES QtSensors_DROPPED_ENTRIES)
