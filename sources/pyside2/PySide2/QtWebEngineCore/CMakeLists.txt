project(QtWebEngineCore)

set(QtWebEngineCore_SRC
${QtWebEngineCore_GEN_DIR}/qwebenginecookiestore_wrapper.cpp
${QtWebEngineCore_GEN_DIR}/qwebenginehttprequest_wrapper.cpp
${QtWebEngineCore_GEN_DIR}/qwebengineurlrequestinfo_wrapper.cpp
${QtWebEngineCore_GEN_DIR}/qwebengineurlrequestinterceptor_wrapper.cpp
${QtWebEngineCore_GEN_DIR}/qwebengineurlrequestjob_wrapper.cpp
${QtWebEngineCore_GEN_DIR}/qwebengineurlschemehandler_wrapper.cpp
# module is always needed
${QtWebEngineCore_GEN_DIR}/qtwebenginecore_module_wrapper.cpp
)

if (Qt5WebEngineCore_VERSION VERSION_EQUAL 5.12.0 OR Qt5WebEngineCore_VERSION VERSION_GREATER 5.12.0)
    list(APPEND QtWebEngineCore_SRC
         ${QtWebEngineCore_GEN_DIR}/qwebengineurlscheme_wrapper.cpp)
endif()

set(QtWebEngineCore_include_dirs
                            ${QtWebEngineCore_SOURCE_DIR}
                            ${QtWebEngineCore_BINARY_DIR}
                            ${Qt5Core_INCLUDE_DIRS}
                            ${libpyside_SOURCE_DIR}
                            ${QtCore_GEN_DIR}
                            )
set(QtWebEngineCore_libraries pyside2
                            ${Qt5WebEngineCore_LIBRARIES}
                            ${Qt5Core_LIBRARIES}
                            )
set(QtWebEngineCore_deps QtCore)
create_pyside_module(NAME QtWebEngineCore
                     INCLUDE_DIRS QtWebEngineCore_include_dirs
                     LIBRARIES QtWebEngineCore_libraries
                     DEPS QtWebEngineCore_deps
                     TYPESYSTEM_PATH QtWebEngineCore_SOURCE_DIR
                     SOURCES QtWebEngineCore_SRC)

