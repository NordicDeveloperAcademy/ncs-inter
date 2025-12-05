# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(devacademy)

# Step 6.1 - Include common files from Zephyr USB samples
target_sources(app PRIVATE src/main.c)
