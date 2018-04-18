project(QtLocation)

set(QtLocation_OPTIONAL_SRC )
set(QtLocation_DROPPED_ENTRIES )

set(QtLocation_SRC
${QtLocation_GEN_DIR}/qgeocodereply_wrapper.cpp
${QtLocation_GEN_DIR}/qgeomaneuver_wrapper.cpp
${QtLocation_GEN_DIR}/qgeoroute_wrapper.cpp
${QtLocation_GEN_DIR}/qgeoroutereply_wrapper.cpp
${QtLocation_GEN_DIR}/qgeoroutesegment_wrapper.cpp
${QtLocation_GEN_DIR}/qgeoserviceprovider_wrapper.cpp
${QtLocation_GEN_DIR}/qplace_wrapper.cpp
${QtLocation_GEN_DIR}/qplacecontentreply_wrapper.cpp
${QtLocation_GEN_DIR}/qplacedetailsreply_wrapper.cpp
${QtLocation_GEN_DIR}/qplaceicon_wrapper.cpp
${QtLocation_GEN_DIR}/qplaceidreply_wrapper.cpp
${QtLocation_GEN_DIR}/qplacemanager_wrapper.cpp
${QtLocation_GEN_DIR}/qplaceuser_wrapper.cpp
${QtLocation_GEN_DIR}/qgeocodingmanager_wrapper.cpp
${QtLocation_GEN_DIR}/qgeocodingmanagerengine_wrapper.cpp
${QtLocation_GEN_DIR}/qgeorouterequest_wrapper.cpp
${QtLocation_GEN_DIR}/qgeoroutingmanager_wrapper.cpp
${QtLocation_GEN_DIR}/qgeoroutingmanagerengine_wrapper.cpp
${QtLocation_GEN_DIR}/qgeoserviceproviderfactory_wrapper.cpp
${QtLocation_GEN_DIR}/qplaceattribute_wrapper.cpp
${QtLocation_GEN_DIR}/qplacecategory_wrapper.cpp
${QtLocation_GEN_DIR}/qplacecontactdetail_wrapper.cpp
${QtLocation_GEN_DIR}/qplacecontent_wrapper.cpp
${QtLocation_GEN_DIR}/qplacecontentrequest_wrapper.cpp
${QtLocation_GEN_DIR}/qplaceeditorial_wrapper.cpp
${QtLocation_GEN_DIR}/qplaceimage_wrapper.cpp
${QtLocation_GEN_DIR}/qplacemanagerengine_wrapper.cpp
${QtLocation_GEN_DIR}/qplacematchreply_wrapper.cpp
${QtLocation_GEN_DIR}/qplacematchrequest_wrapper.cpp
${QtLocation_GEN_DIR}/qplaceproposedsearchresult_wrapper.cpp
${QtLocation_GEN_DIR}/qplaceratings_wrapper.cpp
${QtLocation_GEN_DIR}/qplacereply_wrapper.cpp
${QtLocation_GEN_DIR}/qplaceresult_wrapper.cpp
${QtLocation_GEN_DIR}/qplacereview_wrapper.cpp
${QtLocation_GEN_DIR}/qplacesearchreply_wrapper.cpp
${QtLocation_GEN_DIR}/qplacesearchrequest_wrapper.cpp
${QtLocation_GEN_DIR}/qplacesearchresult_wrapper.cpp
${QtLocation_GEN_DIR}/qplacesearchsuggestionreply_wrapper.cpp
${QtLocation_GEN_DIR}/qplacesupplier_wrapper.cpp
# module is always needed
${QtLocation_GEN_DIR}/qtlocation_module_wrapper.cpp
)

if (Qt5Location_VERSION VERSION_EQUAL 5.11.0 OR Qt5Location_VERSION VERSION_GREATER 5.11.0)
    list(APPEND QtLocation_SRC
         ${QtLocation_GEN_DIR}/qgeoserviceproviderfactoryv2_wrapper.cpp)
endif()

set(QtLocation_include_dirs ${QtLocation_SOURCE_DIR}
                            ${QtLocation_BINARY_DIR}
                            ${Qt5Core_INCLUDE_DIRS}
                            ${Qt5Location_INCLUDE_DIRS}
                            ${SHIBOKEN_INCLUDE_DIR}
                            ${libpyside_SOURCE_DIR}
                            ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                            ${QtCore_GEN_DIR}
                            ${QtPositioning_GEN_DIR})

set(QtLocation_libraries pyside2
                         ${SHIBOKEN_PYTHON_LIBRARIES}
                         ${SHIBOKEN_LIBRARY}
                         ${Qt5Location_LIBRARIES})

set(QtLocation_deps QtCore QtPositioning)

create_pyside_module(QtLocation
                     QtLocation_include_dirs
                     QtLocation_libraries
                     QtLocation_deps
                     QtLocation_SOURCE_DIR
                     QtLocation_SRC
                     ""
                     ""
                     QtLocation_DROPPED_ENTRIES)
