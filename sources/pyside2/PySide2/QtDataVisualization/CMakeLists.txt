project(QtDataVisualization)

set(QtDataVisualization_SRC
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qabstract3daxis_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qcategory3daxis_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qlogvalue3daxisformatter_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qvalue3daxis_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qvalue3daxisformatter_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qabstract3dseries_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qabstractdataproxy_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qbar3dseries_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qbardataitem_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qbardataproxy_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qcustom3ditem_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qcustom3dlabel_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qcustom3dvolume_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qheightmapsurfacedataproxy_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qitemmodelbardataproxy_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qitemmodelscatterdataproxy_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qitemmodelsurfacedataproxy_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qscatter3dseries_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qscatterdataitem_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qscatterdataproxy_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qsurface3dseries_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qsurfacedataitem_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qsurfacedataproxy_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_q3dbars_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_q3dcamera_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_q3dlight_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_q3dobject_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_q3dscatter_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_q3dscene_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_q3dsurface_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qabstract3dgraph_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_q3dinputhandler_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qabstract3dinputhandler_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_qtouch3dinputhandler_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_q3dtheme_wrapper.cpp
${QtDataVisualization_GEN_DIR}/qtdatavisualization_wrapper.cpp
# module is always needed
${QtDataVisualization_GEN_DIR}/qtdatavisualization_module_wrapper.cpp
)

set(QtDataVisualization_include_dirs ${QtDataVisualization_SOURCE_DIR}
                          ${QtDataVisualization_BINARY_DIR}
                          ${Qt5Core_INCLUDE_DIRS}
                          ${Qt5Gui_INCLUDE_DIRS}
                          ${Qt5DataVisualization_INCLUDE_DIRS}
                          ${SHIBOKEN_INCLUDE_DIR}
                          ${libpyside_SOURCE_DIR}
                          ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                          ${QtCore_GEN_DIR}
                          ${QtGui_GEN_DIR})

set(QtDataVisualization_libraries    pyside2
                          ${SHIBOKEN_PYTHON_LIBRARIES}
                          ${SHIBOKEN_LIBRARY}
                          ${Qt5DataVisualization_LIBRARIES}
                          ${Qt5Core_LIBRARIES}
                          ${Qt5Gui_LIBRARIES})

set(QtDataVisualization_deps QtCore QtGui)

create_pyside_module(QtDataVisualization
                     QtDataVisualization_include_dirs
                     QtDataVisualization_libraries
                     QtDataVisualization_deps
                     QtDataVisualization_SOURCE_DIR
                     QtDataVisualization_SRC
                     "")
