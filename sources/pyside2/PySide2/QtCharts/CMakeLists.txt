project(QtCharts)

set(QtCharts_SRC
${QtCharts_GEN_DIR}/qtcharts_qabstractaxis_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qabstractbarseries_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qabstractseries_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qarealegendmarker_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qareaseries_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qbarcategoryaxis_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qbarlegendmarker_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qbarmodelmapper_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qbarseries_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qbarset_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qboxplotlegendmarker_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qboxplotmodelmapper_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qboxplotseries_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qboxset_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qcandlesticklegendmarker_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qcandlestickmodelmapper_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qcandlestickseries_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qcandlestickset_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qcategoryaxis_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qchart_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qchartview_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qdatetimeaxis_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qhbarmodelmapper_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qhboxplotmodelmapper_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qhcandlestickmodelmapper_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qhorizontalbarseries_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qhorizontalpercentbarseries_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qhorizontalstackedbarseries_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qhpiemodelmapper_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qhxymodelmapper_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qlegend_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qlegendmarker_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qlineseries_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qlogvalueaxis_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qpercentbarseries_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qpielegendmarker_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qpiemodelmapper_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qpieseries_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qpieslice_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qpolarchart_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qscatterseries_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qsplineseries_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qstackedbarseries_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qvalueaxis_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qvbarmodelmapper_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qvboxplotmodelmapper_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qvcandlestickmodelmapper_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qvpiemodelmapper_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qvxymodelmapper_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qxylegendmarker_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qxymodelmapper_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_qxyseries_wrapper.cpp
${QtCharts_GEN_DIR}/qtcharts_wrapper.cpp
# module is always needed
${QtCharts_GEN_DIR}/qtcharts_module_wrapper.cpp
)

make_path(QtCharts_typesystem_path
    ${QtCore_SOURCE_DIR}
    ${QtCore_BINARY_DIR}
    ${QtGui_SOURCE_DIR}
    ${QtGui_BINARY_DIR}
    ${QtWidgets_SOURCE_DIR}
    ${QtWidgets_BINARY_DIR}
    ${QtCharts_SOURCE_DIR})

set(QtCharts_include_dirs ${QtCharts_SOURCE_DIR}
                          ${QtCharts_BINARY_DIR}
                          ${Qt5Core_INCLUDE_DIRS}
                          ${Qt5Gui_INCLUDE_DIRS}
                          ${Qt5Widgets_INCLUDE_DIRS}
                          ${Qt5Charts_INCLUDE_DIRS}
                          ${SHIBOKEN_INCLUDE_DIR}
                          ${libpyside_SOURCE_DIR}
                          ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                          ${QtCore_GEN_DIR}
                          ${QtGui_GEN_DIR}
                          ${QtWidgets_GEN_DIR})

set(QtCharts_libraries    pyside2
                          ${SHIBOKEN_PYTHON_LIBRARIES}
                          ${SHIBOKEN_LIBRARY}
                          ${Qt5Charts_LIBRARIES}
                          ${Qt5Core_LIBRARIES}
                          ${Qt5Gui_LIBRARIES}
                          ${Qt5Widgets_LIBRARIES})

set(QtCharts_deps QtCore QtGui QtWidgets)

create_pyside_module(QtCharts
                     QtCharts_include_dirs
                     QtCharts_libraries
                     QtCharts_deps
                     QtCharts_typesystem_path
                     QtCharts_SRC
                     "")
