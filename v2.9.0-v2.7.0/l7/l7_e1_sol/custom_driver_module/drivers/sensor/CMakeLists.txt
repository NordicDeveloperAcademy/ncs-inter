#STEP 6.3: Add the custom_bme280 directory as a subdirectory in the sensor driver
add_subdirectory_ifdef(CONFIG_CUSTOM_BME280 custom_bme280)