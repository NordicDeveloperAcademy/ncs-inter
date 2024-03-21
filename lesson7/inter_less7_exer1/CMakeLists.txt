# SPDX-License-Identifier: Apache-2.0

# STEP 2 - Add the driver directory as a Zephyr module

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nrf_connect_sdk_intermediate)

target_sources(app PRIVATE  src/main.c)
