#STEP 7.2: Create CMakeLists.txt for our module in the root directory

cmake_minimum_required(VERSION 3.21)
# Subdirectories
add_subdirectory(drivers)

# Include headers
zephyr_include_directories(drivers)