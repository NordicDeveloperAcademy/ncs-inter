# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)
# Subdirectories
add_subdirectory(drivers)

# STEP 2.6 Include header to syscalls list
zephyr_syscall_include_directories(include)

zephyr_include_directories(drivers)
zephyr_include_directories(include)