#STEP 6.1: Populate the CMakeLists.txt file for the custom driver
zephyr_library()
zephyr_library_sources(custom_bme280.c)