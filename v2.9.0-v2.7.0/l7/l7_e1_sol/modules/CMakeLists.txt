# SPDX-License-Identifier: Apache-2.0

# Point to this project's root directory.
set(CUSTOM_BME280_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE PATH "CUSTOM BME DIR")

# Subdirectories
add_subdirectory(drivers)

# Include headers
zephyr_include_directories(drivers)