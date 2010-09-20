project(sample)

set(sample_TYPESYSTEM
${CMAKE_CURRENT_SOURCE_DIR}/typesystem_sample.xml
)

set(sample_SRC
${CMAKE_CURRENT_BINARY_DIR}/sample/abstractmodifications_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/abstract_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/base1_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/base2_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/base3_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/base4_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/base5_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/base6_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/blackbox_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/bucket_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/collector_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/derived_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/derived_someinnerclass_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/echo_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/event_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/implicitconv_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/implicitbase_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/implicittarget_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/intlist_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/sortedoverload_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/intwrapper_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/injectcode_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/listuser_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/mapuser_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/mderived1_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/mderived2_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/mderived3_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/mderived4_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/mderived5_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/modifications_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/modifiedconstructor_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/noimplicitconversion_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/nondefaultctor_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/objecttype_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/objecttypelayout_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/objectview_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/oddbooluser_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/overload_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/pairuser_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/point_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/pointerholder_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/polygon_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/privatector_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/privatedtor_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/protectedenumclass_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/protectednonpolymorphic_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/protectedpolymorphic_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/protectedpolymorphicdaughter_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/protectedpolymorphicgranddaughter_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/protectedproperty_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/protectedvirtualdestructor_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/reference_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/sample_module_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/samplenamespace_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/samplenamespace_someclass_someinnerclass_okthisisrecursiveenough_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/samplenamespace_someclass_someinnerclass_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/samplenamespace_someclass_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/samplenamespace_derivedfromnamespace_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/simplefile_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/size_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/sonofmderived1_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/str_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/strlist_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/time_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/virtualdaughter_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/virtualdtor_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/virtualmethods_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/voidholder_wrapper.cpp
${CMAKE_CURRENT_BINARY_DIR}/sample/valueandvirtual_wrapper.cpp
)

add_custom_command(OUTPUT ${sample_SRC}
COMMAND ${GENERATORRUNNER_BINARY} --generatorSet=${generators_BINARY_DIR}/shiboken_generator${CMAKE_RELEASE_POSTFIX}${CMAKE_DEBUG_POSTFIX} --enable-parent-ctor-heuristic
        ${CMAKE_CURRENT_SOURCE_DIR}/global.h
        --include-paths=${libsample_SOURCE_DIR}
        --typesystem-paths=${CMAKE_CURRENT_SOURCE_DIR}
        --output-directory=${CMAKE_CURRENT_BINARY_DIR}
        ${sample_TYPESYSTEM}
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
COMMENT "Running generator for test binding..."
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}
                    ${PYTHON_INCLUDE_PATH}
                    ${libsample_SOURCE_DIR}
                    ${libshiboken_SOURCE_DIR})
add_library(sample MODULE ${sample_SRC})
set_property(TARGET sample PROPERTY PREFIX "")
if(WIN32)
    set_property(TARGET sample PROPERTY SUFFIX ".pyd")
endif()
target_link_libraries(sample
                      libsample
                      ${SBK_PYTHON_LIBRARIES}
                      libshiboken)

add_dependencies(sample shiboken_generator)

