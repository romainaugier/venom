# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2025 - Present Romain Augier
# All rights reserved.

include(CMakePrintHelpers)

cmake_minimum_required(VERSION 3.15)

project(venom VERSION ${VERSION})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_VERBOSE_MAKEFILE ON)

set(LIB_NAME libvenom)
set(APP_NAME venomc)

if(RUN_TESTS EQUAL 1)
    message(STATUS "RUN_TESTS enabled, building and running tests")
    set(TESTS_DATA_DIR "\"${CMAKE_SOURCE_DIR}/tests_data\"")
    enable_testing()
endif()

set(libromano_DIR "${CMAKE_SOURCE_DIR}/libromano/install/cmake")
find_package(libromano REQUIRED)

add_subdirectory(src)

if(RUN_TESTS EQUAL 1)
    add_subdirectory(tests)
endif()