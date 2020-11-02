cmake_minimum_required(VERSION 3.16)
project(pyside-tools)

# Don't display "up-to-date / install" messages when installing, to reduce visual clutter.
if (QUIET_BUILD)
    set(CMAKE_INSTALL_MESSAGE NEVER)
endif()

find_package(Qt6 COMPONENTS Core)

set(TOOLS_PATH "${QT6_INSTALL_PREFIX}/bin")

set(UIC_PATH "${TOOLS_PATH}/uic${CMAKE_EXECUTABLE_SUFFIX}")
set(RCC_PATH "${TOOLS_PATH}/rcc${CMAKE_EXECUTABLE_SUFFIX}")
if (APPLE)
    set(DESIGNER_PATH "${TOOLS_PATH}/Designer.app")
else()
    set(DESIGNER_PATH "${TOOLS_PATH}/designer${CMAKE_EXECUTABLE_SUFFIX}")
endif()

install(FILES "${UIC_PATH}"
        DESTINATION bin
        PERMISSIONS
        OWNER_EXECUTE OWNER_WRITE OWNER_READ
        GROUP_EXECUTE GROUP_READ
        WORLD_EXECUTE WORLD_READ)

install(FILES "${RCC_PATH}"
        DESTINATION bin
        PERMISSIONS
        OWNER_EXECUTE OWNER_WRITE OWNER_READ
        GROUP_EXECUTE GROUP_READ
        WORLD_EXECUTE WORLD_READ)

if (EXISTS ${DESIGNER_PATH})
    if (APPLE)
        install(DIRECTORY "${DESIGNER_PATH}"
                DESTINATION bin
                FILE_PERMISSIONS
                OWNER_EXECUTE OWNER_WRITE OWNER_READ
                GROUP_EXECUTE GROUP_READ
                WORLD_EXECUTE WORLD_READ)
    else()
        install(FILES "${DESIGNER_PATH}"
                DESTINATION bin
                PERMISSIONS
                OWNER_EXECUTE OWNER_WRITE OWNER_READ
                GROUP_EXECUTE GROUP_READ
                WORLD_EXECUTE WORLD_READ)
    endif()
endif()

# pyside6-rcc, pyside6-uic, pyside6-designer, shiboken and pyside6-lupdate entrypoints
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pyside_tool.py
        DESTINATION bin
        PERMISSIONS
        OWNER_EXECUTE OWNER_WRITE OWNER_READ
        GROUP_EXECUTE GROUP_READ
        WORLD_EXECUTE WORLD_READ)
