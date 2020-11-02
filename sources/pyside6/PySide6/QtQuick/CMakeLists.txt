project(QtQuick)

set(QtQuick_registerType "${QtQuick_SOURCE_DIR}/pysidequickregistertype.cpp")

set(QtQuick_SRC
${QtQuick_GEN_DIR}/qquickasyncimageprovider_wrapper.cpp
${QtQuick_GEN_DIR}/qquickframebufferobject_wrapper.cpp
${QtQuick_GEN_DIR}/qquickframebufferobject_renderer_wrapper.cpp
${QtQuick_GEN_DIR}/qquicktexturefactory_wrapper.cpp
${QtQuick_GEN_DIR}/qquickimageprovider_wrapper.cpp
${QtQuick_GEN_DIR}/qquickimageresponse_wrapper.cpp
${QtQuick_GEN_DIR}/qquicktransform_wrapper.cpp
${QtQuick_GEN_DIR}/qquickitem_wrapper.cpp
${QtQuick_GEN_DIR}/qquickitem_updatepaintnodedata_wrapper.cpp
${QtQuick_GEN_DIR}/qquickitemgrabresult_wrapper.cpp
${QtQuick_GEN_DIR}/qsharedpointer_qquickitemgrabresult_wrapper.cpp
${QtQuick_GEN_DIR}/qquickpainteditem_wrapper.cpp
${QtQuick_GEN_DIR}/qquickrendercontrol_wrapper.cpp
${QtQuick_GEN_DIR}/qquicktextdocument_wrapper.cpp
${QtQuick_GEN_DIR}/qquickview_wrapper.cpp
${QtQuick_GEN_DIR}/qquickwindow_wrapper.cpp
${QtQuick_GEN_DIR}/qsgbasicgeometrynode_wrapper.cpp
${QtQuick_GEN_DIR}/qsgclipnode_wrapper.cpp
${QtQuick_GEN_DIR}/qsgdynamictexture_wrapper.cpp
#${QtQuick_GEN_DIR}/qsgflatcolormaterial_wrapper.cpp
${QtQuick_GEN_DIR}/qsggeometry_attribute_wrapper.cpp
${QtQuick_GEN_DIR}/qsggeometry_attributeset_wrapper.cpp
${QtQuick_GEN_DIR}/qsggeometry_coloredpoint2d_wrapper.cpp
${QtQuick_GEN_DIR}/qsggeometry_point2d_wrapper.cpp
${QtQuick_GEN_DIR}/qsggeometry_texturedpoint2d_wrapper.cpp
${QtQuick_GEN_DIR}/qsggeometry_wrapper.cpp
${QtQuick_GEN_DIR}/qsggeometrynode_wrapper.cpp
#${QtQuick_GEN_DIR}/qsgmaterial_wrapper.cpp
#${QtQuick_GEN_DIR}/qsgmaterialshader_renderstate_wrapper.cpp
# Issue with virtual char const *const *attributeNames()
#${QtQuick_GEN_DIR}/qsgmaterialshader_wrapper.cpp
${QtQuick_GEN_DIR}/qsgmaterialtype_wrapper.cpp
${QtQuick_GEN_DIR}/qsgnode_wrapper.cpp
${QtQuick_GEN_DIR}/qsgopacitynode_wrapper.cpp
#${QtQuick_GEN_DIR}/qsgopaquetexturematerial_wrapper.cpp
#${QtQuick_GEN_DIR}/qsgsimplematerial_wrapper.cpp
#${QtQuick_GEN_DIR}/qsgsimplematerialshader_wrapper.cpp
${QtQuick_GEN_DIR}/qsgsimplerectnode_wrapper.cpp
${QtQuick_GEN_DIR}/qsgsimpletexturenode_wrapper.cpp
${QtQuick_GEN_DIR}/qsgtexture_wrapper.cpp
#${QtQuick_GEN_DIR}/qsgtexturematerial_wrapper.cpp
${QtQuick_GEN_DIR}/qsgtextureprovider_wrapper.cpp
${QtQuick_GEN_DIR}/qsgtransformnode_wrapper.cpp
#${QtQuick_GEN_DIR}/qsgvertexcolormaterial_wrapper.cpp
# module is always needed
${QtQuick_GEN_DIR}/qtquick_module_wrapper.cpp
)

set(QtQuick_include_dirs  ${QtQuick_SOURCE_DIR}
                          ${QtQml_SOURCE_DIR}
                          ${Qt${QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
                          ${Qt${QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
                          ${Qt${QT_MAJOR_VERSION}Network_INCLUDE_DIRS}
                          ${Qt${QT_MAJOR_VERSION}Qml_INCLUDE_DIRS}
                          ${Qt${QT_MAJOR_VERSION}Quick_INCLUDE_DIRS}
                          ${libpyside_SOURCE_DIR}
                          ${QtGui_GEN_DIR}
                          ${QtCore_GEN_DIR}
                          ${QtNetwork_GEN_DIR}
                          ${QtQml_GEN_DIR}
                          ${QtQuick_GEN_DIR})

set(QtQuick_libraries pyside6
                      ${Qt${QT_MAJOR_VERSION}Core_LIBRARIES}
                      ${Qt${QT_MAJOR_VERSION}Gui_LIBRARIES}
                      ${Qt${QT_MAJOR_VERSION}Network_LIBRARIES}
                      ${Qt${QT_MAJOR_VERSION}Qml_LIBRARIES}
                      ${Qt${QT_MAJOR_VERSION}Quick_LIBRARIES})

set(QtQuick_deps QtGui QtNetwork QtQml)

create_pyside_module(NAME QtQuick
                     INCLUDE_DIRS QtQuick_include_dirs
                     LIBRARIES QtQuick_libraries
                     DEPS QtQuick_deps
                     TYPESYSTEM_PATH QtQuick_SOURCE_DIR
                     SOURCES QtQuick_SRC
                     STATIC_SOURCES QtQuick_registerType)
