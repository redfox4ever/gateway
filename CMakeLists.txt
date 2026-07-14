# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
set(BOARD stm32f4_disco)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(gateway)

include(${ZEPHYR_BASE}/samples/subsys/usb/common/common.cmake)
target_include_directories(app PRIVATE src/)
FILE(GLOB gateway_sources src/main.c)
target_sources(app PRIVATE ${gateway_sources})
