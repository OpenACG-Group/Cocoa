file(GLOB INTERNAL_RESOURCES_FILES ${INTERNAL_RESOURCES_DIR}/*)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/@internal.crpkg
    COMMAND ${CMAKE_BINARY_DIR}/crpkg-packager ${CMAKE_BINARY_DIR}/@internal.crpkg ${INTERNAL_RESOURCES_DIR}
    DEPENDS crpkg-packager ${INTERNAL_RESOURCES_FILES}
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/internal-datas.c
    COMMAND ${CMAKE_HOME_DIRECTORY}/script/intern-resource-package.py ${CMAKE_BINARY_DIR}/@internal.crpkg
            > ${CMAKE_CURRENT_BINARY_DIR}/internal-datas.c
    DEPENDS ${CMAKE_BINARY_DIR}/@internal.crpkg
            ${CMAKE_HOME_DIRECTORY}/script/intern-resource-package.py
)

add_library(
        CRPKG STATIC

        CRPKG.h
        Protocol.h
        Protocol.cc
        Composer.h
        Composer.cc
        VirtualDisk.h
        VirtualDisk.cc
        ResourceManager.h
        ResourceManager.cc
        ${CMAKE_CURRENT_BINARY_DIR}/internal-datas.c
)

add_executable(
        crpkg-packager

        crpkg-packager.cc
        CRPKG.h
        Composer.h
        Composer.cc
        Protocol.h
        Protocol.cc
)

target_link_libraries(crpkg-packager PRIVATE Core ${LINK_STATIC_FMT} ${LINK_STATIC_LIBUV})
set_target_properties(crpkg-packager PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
