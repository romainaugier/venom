# SPDX-License-Identifier: BSD-3-Clause 
# Copyright (c) 2025 - Present Romain Augier
# All rights reserved. 

include(GNUInstallDirs)
include(target_options)
include(utils)

file(GLOB_RECURSE src_files *.c)

list(REMOVE_ITEM src_files "main.c")

# libvenom

add_library(${LIB_NAME} SHARED ${src_files})
set_target_properties(${LIB_NAME} PROPERTIES PREFIX "")
set_target_properties(${LIB_NAME} PROPERTIES C_STANDARD 99)
set_target_properties(${LIB_NAME} PROPERTIES DEBUG_POSTFIX "d")

message(STATUS ${LIB_NAME})

set_target_options(${LIB_NAME})

if(WIN32)
    set_target_properties(${LIB_NAME} PROPERTIES CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

target_compile_definitions(${LIB_NAME} PUBLIC VENOM_BUILD_SHARED)

target_include_directories(${LIB_NAME}
                           PUBLIC
                           $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(${LIB_NAME} PUBLIC libromano::libromano)

# venomc

add_executable(${APP_NAME} main.c)
set_target_properties(${APP_NAME} PROPERTIES PREFIX "")
set_target_properties(${APP_NAME} PROPERTIES C_STANDARD 99)
set_target_properties(${APP_NAME} PROPERTIES DEBUG_POSTFIX "d")

set_target_options(${APP_NAME})

target_link_libraries(${APP_NAME} PRIVATE ${LIB_NAME})

install(
    TARGETS ${LIB_NAME} ${APP_NAME}
    EXPORT "${PROJECT_NAME}Targets"
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

install(
    EXPORT "${PROJECT_NAME}Targets"
    FILE "${PROJECT_NAME}Targets.cmake"
    NAMESPACE "${PROJECT_NAME}::"
    DESTINATION cmake
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_BINDIR
    INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake
)

write_basic_package_version_file(
    "${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION cmake
)