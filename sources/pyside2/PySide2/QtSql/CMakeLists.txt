project(QtSql)

set(QtSql_SRC
${QtSql_GEN_DIR}/qsql_wrapper.cpp
${QtSql_GEN_DIR}/qsqldatabase_wrapper.cpp
${QtSql_GEN_DIR}/qsqldriver_wrapper.cpp
${QtSql_GEN_DIR}/qsqldrivercreatorbase_wrapper.cpp
${QtSql_GEN_DIR}/qsqlerror_wrapper.cpp
${QtSql_GEN_DIR}/qsqlfield_wrapper.cpp
${QtSql_GEN_DIR}/qsqlindex_wrapper.cpp
${QtSql_GEN_DIR}/qsqlquery_wrapper.cpp
${QtSql_GEN_DIR}/qsqlquerymodel_wrapper.cpp
${QtSql_GEN_DIR}/qsqlrecord_wrapper.cpp
${QtSql_GEN_DIR}/qsqlrelation_wrapper.cpp
${QtSql_GEN_DIR}/qsqlrelationaldelegate_wrapper.cpp
${QtSql_GEN_DIR}/qsqlrelationaltablemodel_wrapper.cpp
${QtSql_GEN_DIR}/qsqlresult_wrapper.cpp
${QtSql_GEN_DIR}/qsqltablemodel_wrapper.cpp
# module is always needed
${QtSql_GEN_DIR}/qtsql_module_wrapper.cpp
)

configure_file("${QtSql_SOURCE_DIR}/QtSql_global.pre.h.in"
               "${QtSql_BINARY_DIR}/QtSql_global.pre.h" @ONLY)

set(QtSql_include_dirs  ${QtSql_SOURCE_DIR}
                        ${QtSql_BINARY_DIR}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${Qt5Gui_INCLUDE_DIRS}
                        ${Qt5Widgets_INCLUDE_DIRS}
                        ${Qt5Sql_INCLUDE_DIRS}
                        ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                        ${SHIBOKEN_INCLUDE_DIR}
                        ${libpyside_SOURCE_DIR}
                        ${QtCore_GEN_DIR}
                        ${QtGui_GEN_DIR}
                        ${QtWidgets_GEN_DIR}
                        )
set(QtSql_libraries     pyside2
                        ${SHIBOKEN_PYTHON_LIBRARIES}
                        ${SHIBOKEN_LIBRARY}
                        ${Qt5Core_LIBRARIES}
                        ${Qt5Gui_LIBRARIES}
                        ${Qt5Widgets_LIBRARIES}
                        ${Qt5Sql_LIBRARIES})
set(QtSql_deps QtWidgets)

create_pyside_module(NAME QtSql
                     INCLUDE_DIRS QtSql_include_dirs
                     LIBRARIES QtSql_libraries
                     DEPS QtSql_deps
                     TYPESYSTEM_PATH QtSql_SOURCE_DIR
                     SOURCES QtSql_SRC)
