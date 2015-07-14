project(QtSql)

set(QtSql_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSql/qsqlindex_wrapper.cpp

    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSql/qsqldatabase_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSql/qsqldrivercreatorbase_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSql/qsqldriver_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSql/qsqlerror_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSql/qsqlfield_wrapper.cpp

    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSql/qsqlquerymodel_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSql/qsqlquery_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSql/qsqlrecord_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSql/qsqlrelationaldelegate_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSql/qsqlrelationaltablemodel_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSql/qsqlrelation_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSql/qsqlresult_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSql/qsqltablemodel_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSql/qsql_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSql/qtsql_module_wrapper.cpp
)

# fake these variables for qt5
set(QtSql_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(QtSql_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSql)

set(QtSql_typesystem_path "${QtCore_SOURCE_DIR}${PATH_SEP}${QtCore_BINARY_DIR}${PATH_SEP}${QtGui_SOURCE_DIR}${PATH_SEP}${QtGui_BINARY_DIR}${PATH_SEP}${QtWidgets_SOURCE_DIR}${PATH_SEP}${QtWidgets_BINARY_DIR}${PATH_SEP}${QtSql_SOURCE_DIR}")
set(QtSql_include_dirs  ${CMAKE_CURRENT_SOURCE_DIR}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${Qt5Gui_INCLUDE_DIRS}
                        ${Qt5Widgets_INCLUDE_DIRS}
                        ${Qt5Sql_INCLUDE_DIRS}
                        ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                        ${SHIBOKEN_INCLUDE_DIR}
                        ${libpyside_SOURCE_DIR}
                        ${QtCore_BINARY_DIR}/PySide/QtCore/
                        ${QtGui_BINARY_DIR}/PySide/QtGui/
                        ${QtWidgets_BINARY_DIR}/PySide/QtWidgets/
                        ${CMAKE_CURRENT_BINARY_DIR}/PySide/QtSql/)
set(QtSql_libraries     pyside
                        ${SHIBOKEN_PYTHON_LIBRARIES}
                        ${SHIBOKEN_LIBRARY}
                        ${Qt5Core_LIBRARIES}
                        ${Qt5Gui_LIBRARIES}
                        ${Qt5Widgets_LIBRARIES}
                        ${Qt5Sql_LIBRARIES})
set(QtSql_deps QtWidgets)
create_pyside_module(QtSql
                     QtSql_include_dirs
                     QtSql_libraries
                     QtSql_deps
                     QtSql_typesystem_path
                     QtSql_SRC
                     "")
