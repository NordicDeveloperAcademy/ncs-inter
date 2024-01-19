/*
 * Copyright (c) 2019 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "custom_bme280_driver_spi.h"
#include <zephyr/types.h>
#include <zephyr/sys/printk.h>
#include <zephyr/syscall_handler.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

/* Obtain the SPI Controller node and GPIO node for CS pin */
static const struct device *spidev = DEVICE_DT_GET(DT_NODELABEL(spi1));
const struct device *gpiodev =DEVICE_DT_GET(DT_NODELABEL(gpio0));

//Define pin number for chip-select-bar pin
#define CSB_PIN 30

/**
 * This is a minimal example of data that can be passed into device driver model
 */

static struct custom_driver_data {
	uint32_t foo;
} data;

const struct spi_config bme_spi_config = {
	.frequency = 125000U,
	.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
	.slave = 0,
};

//Data structure to store BME280 data
struct bme280_data {
	/* Compensation parameters. */
	uint16_t dig_t1;
	int16_t dig_t2;
	int16_t dig_t3;
	uint16_t dig_p1;
	int16_t dig_p2;
	int16_t dig_p3;
	int16_t dig_p4;
	int16_t dig_p5;
	int16_t dig_p6;
	int16_t dig_p7;
	int16_t dig_p8;
	int16_t dig_p9;
	uint8_t dig_h1;
	int16_t dig_h2;
	uint8_t dig_h3;
	int16_t dig_h4;
	int16_t dig_h5;
	int8_t dig_h6;

	/* Compensated values. */
	int32_t comp_temp;
	uint32_t comp_press;
	uint32_t comp_humidity;

	/* Carryover between temperature and pressure/humidity compensation. */
	int32_t t_fine;

	uint8_t chip_id;
}bmedata;
/*
 @brief The function to read-out a single BME280 register
 @param reg is the address of the register
 @param data is the pointer to store read data
 @param size is the size of data
 @return 0 or err code
*/
static int bme_read_reg(uint8_t reg, uint8_t *data, int size)
{
	int err;

	uint8_t tx_buffer = reg;

	/* Set the transmit and receive buffers */
	struct spi_buf tx_spi_buf = {.buf = (void *)&tx_buffer, .len = 1};	
	struct spi_buf_set tx_spi_buf_set = {.buffers = &tx_spi_buf, .count = 1};
	struct spi_buf rx_spi_bufs = {.buf = data, .len = size};
	struct spi_buf_set rx_spi_buffer_set = {.buffers = &rx_spi_bufs, .count = 1};

	/* Enable the sensor by activating Chip-select (put CSB into active state) */
	gpio_pin_set(gpiodev, CSB_PIN, 1);

	/* Write the transmit buffers to sensor using spi_write() */
	err = spi_write(spidev, &bme_spi_config, &tx_spi_buf_set);
	if (err < 0) {
		printk("\nspi_write FAIL: Error %d\n", err);
		return err;
	}

	/* Read the data from sensor using spi_read() */
	err = spi_read(spidev, &bme_spi_config, &rx_spi_buffer_set);
	if (err < 0) {
		printk("\nspi_read FAIL: Error %d\n", err);
		return err;
	}

	/* Disable the device by putting CSB into inactive state */
	gpio_pin_set(gpiodev, CSB_PIN, 0);

	return 0;
}

/// @brief The function to write a value to the BME280 register
/// @param reg is the address of the register
/// @param values is the data to write
/// @return 0 or err code
static int bme_write_reg(uint8_t reg, uint8_t value)
{
	int err;

	//Bit7 is 0 for the write command
	uint8_t tx_buf[] = {(reg & 0x7F), value};

	/*Step 8: Set tx-buffer, enable CS, do spi_write() and then disable CS*/
	struct spi_buf tx_spi_buf = {.buf = tx_buf, .len = sizeof(tx_buf)};
	struct spi_buf_set tx_spi_buf_set = {.buffers = &tx_spi_buf, .count = 1};
	gpio_pin_set(gpiodev, CSB_PIN, 1);
	err = spi_write(spidev, &bme_spi_config, &tx_spi_buf_set);
	if (err < 0) {
		printk("\nspi_write FAIL: Error %d\n", err);
		return err;
	}
	gpio_pin_set(gpiodev, CSB_PIN, 0);

	return 0;
}

/* @brief The function to read calibration registers and 
        set the values in bme_data struct so that 
        calibrating functiions could use these values 
*/
void bme_calibrationdata_read(void){
	uint8_t values[2];
	uint8_t regaddr;

	/* Reading required number of bytes from respective register location
	and setting data in proper order to form compensation parameter values*/
	regaddr = 0x88;
	bme_read_reg(regaddr, values, 2);
	bmedata.dig_t1 = ((uint16_t)values[1])<<8 | values[0];

	regaddr += 2;
	bme_read_reg(regaddr, values, 2);
	bmedata.dig_t2 = ((uint16_t)values[1])<<8 | values[0];

	regaddr += 2;
	bme_read_reg(regaddr, values, 2);
	bmedata.dig_t3 = ((uint16_t)values[1])<<8 | values[0];

	regaddr += 2;
	bme_read_reg(regaddr, values, 2);
	bmedata.dig_p1 = ((uint16_t)values[1])<<8 | values[0];

	regaddr += 2;
	bme_read_reg(regaddr, values, 2);
	bmedata.dig_p2 = ((uint16_t)values[1])<<8 | values[0];

	regaddr += 2;
	bme_read_reg(regaddr, values, 2);
	bmedata.dig_p3 = ((uint16_t)values[1])<<8 | values[0];

	regaddr += 2;
	bme_read_reg(regaddr, values, 2);
	bmedata.dig_p4 = ((uint16_t)values[1])<<8 | values[0];

	regaddr += 2;
	bme_read_reg(regaddr, values, 2);
	bmedata.dig_p5 = ((uint16_t)values[1])<<8 | values[0];

	regaddr += 2;
	bme_read_reg(regaddr, values, 2);
	bmedata.dig_p6 = ((uint16_t)values[1])<<8 | values[0];

	regaddr += 2;
	bme_read_reg(regaddr, values, 2);
	bmedata.dig_p7 = ((uint16_t)values[1])<<8 | values[0];

	regaddr += 2;
	bme_read_reg(regaddr, values, 2);
	bmedata.dig_p8 = ((uint16_t)values[1])<<8 | values[0];

	regaddr += 2;
	bme_read_reg(regaddr, values, 2);
	bmedata.dig_p9 = ((uint16_t)values[1])<<8 | values[0];

	regaddr += 2;
	bme_read_reg(regaddr, values, 1);
	bmedata.dig_h1 = values[0];

	regaddr += 40;
	bme_read_reg(regaddr, values, 2);
	bmedata.dig_h2 = ((uint16_t)values[1])<<8 | values[0];

	regaddr += 2;
	bme_read_reg(regaddr, values, 1);
	bmedata.dig_h3 = values[0];

	regaddr += 1;
	bme_read_reg(regaddr, values, 2);
	bmedata.dig_h4 = ((uint16_t)values[0])<<4 | (values[1] & 0x0F);

	regaddr += 2;
	bme_read_reg(regaddr, values, 2);
	bmedata.dig_h5 = ((uint16_t)values[1])<<4 | ((values[0] >> 4) & 0x0F);

	regaddr += 2;
	bme_read_reg(regaddr, values, 1);
	bmedata.dig_h6 = values[0];

	printk("BME280 Calibration Data from sensor BME280 read and saved \n");
	printk("----------------------------------------------------------\n");
	printk("\nT1: %d\tT2: %d\tT3: %d", bmedata.dig_t1, bmedata.dig_t2, bmedata.dig_t3);
	printk("\nP1: %d\tP2: %d\tP3: %d", bmedata.dig_p1, bmedata.dig_p2, bmedata.dig_p3);
	printk("\nP4: %d\tP5: %d\tP5: %d", bmedata.dig_p4, bmedata.dig_p5, bmedata.dig_p6);
	printk("\nP7: %d\tP8: %d\tP9: %d", bmedata.dig_p7, bmedata.dig_p8, bmedata.dig_p9);
	printk("\nH1: %d\tH2: %d\tH3: %d", bmedata.dig_h1, bmedata.dig_h2, bmedata.dig_h3);
	printk("\nH4: %d\tH5: %d\tH6: %d\n\n", bmedata.dig_h4, bmedata.dig_h5, bmedata.dig_h6);
}

static int init(const struct device *dev)
{
	int ret;
	data.foo = 5; /* data initialised to something */

	ret = device_is_ready(spidev);
	if (!ret) {
		printk("SPI DEV NOT Ready: Error %d\n", ret);
		return 0;
	}

	ret = device_is_ready(gpiodev);
	if (!ret) {
		printk("GPIO DEV NOT Ready: Error %d\n", ret);
		return 0;
	}

	//Configure CSB pin and LED0
	gpio_pin_configure(gpiodev, CSB_PIN, GPIO_OUTPUT | GPIO_ACTIVE_LOW);
	gpio_pin_configure(gpiodev, 13, GPIO_OUTPUT_ACTIVE);

	//Read calibration data
	bme_calibrationdata_read();

	//Write sampling parameters
	bme_write_reg(0xF2, 0x04);
	bme_write_reg(0xF4, 0x93);

	return 0;
}

static void bme280_compensate_temp(struct bme280_data *data, int32_t adc_temp)
{
	int32_t var1, var2;

	var1 = (((adc_temp >> 3) - ((int32_t)data->dig_t1 << 1)) *
		((int32_t)data->dig_t2)) >> 11;
	var2 = (((((adc_temp >> 4) - ((int32_t)data->dig_t1)) *
		  ((adc_temp >> 4) - ((int32_t)data->dig_t1))) >> 12) *
		((int32_t)data->dig_t3)) >> 14;

	data->t_fine = var1 + var2;
	data->comp_temp = (data->t_fine * 5 + 128) >> 8;

}

static void bme280_compensate_press(struct bme280_data *data, int32_t adc_press)
{
	int64_t var1, var2, p;

	var1 = ((int64_t)data->t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)data->dig_p6;
	var2 = var2 + ((var1 * (int64_t)data->dig_p5) << 17);
	var2 = var2 + (((int64_t)data->dig_p4) << 35);
	var1 = ((var1 * var1 * (int64_t)data->dig_p3) >> 8) +
		((var1 * (int64_t)data->dig_p2) << 12);
	var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)data->dig_p1) >> 33;

	/* Avoid exception caused by division by zero. */
	if (var1 == 0) {
		data->comp_press = 0U;
		return;
	}

	p = 1048576 - adc_press;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = (((int64_t)data->dig_p9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((int64_t)data->dig_p8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)data->dig_p7) << 4);

	data->comp_press = (uint32_t)p;
}

static void bme280_compensate_humidity(struct bme280_data *data, int32_t adc_humidity)
{
	int32_t h;

	h = (data->t_fine - ((int32_t)76800));
	h = ((((adc_humidity << 14) - (((int32_t)data->dig_h4) << 20) -
		(((int32_t)data->dig_h5) * h)) + ((int32_t)16384)) >> 15) *
		(((((((h * ((int32_t)data->dig_h6)) >> 10) * (((h *
		((int32_t)data->dig_h3)) >> 11) + ((int32_t)32768))) >> 10) +
		((int32_t)2097152)) * ((int32_t)data->dig_h2) + 8192) >> 14);
	h = (h - (((((h >> 15) * (h >> 15)) >> 7) *
		((int32_t)data->dig_h1)) >> 4));
	h = (h > 419430400 ? 419430400 : h);

	data->comp_humidity = (uint32_t)(h >> 12);
}

/// @brief The function to read, compensate and show the
///        Temperature, Pressure, and Humidity values from
///        the BME280 device
int bme_read_values(void)
{
	//Set the registers addresses as per data-sheet

	uint8_t regs[] = {0xF7, 0xF8, 0xF9, \
					  0xFA, 0xFB, 0xFC, \
					  0xFD, 0xFE, 0xFF};
	uint8_t readbuf[sizeof(regs)];
	uint8_t size = sizeof(regs);
	int32_t datap = 0, datat = 0, datah = 0;
	int err;

	struct spi_buf tx_spi_buf = {.buf = (void *)&regs, .len = sizeof(regs)};
	struct spi_buf_set tx_spi_buf_set = {.buffers = &tx_spi_buf, .count = 1};
	struct spi_buf rx_spi_bufs = {.buf = readbuf, .len = size};
	struct spi_buf_set rx_spi_buffer_set = {.buffers = &rx_spi_bufs, .count = 1};

	gpio_pin_set(gpiodev, CSB_PIN, 1);
	err = spi_transceive(spidev, &bme_spi_config, &tx_spi_buf_set, &rx_spi_buffer_set);	
	if (err < 0) {
		printk("\nSPI TRANSCEIVE FAILED: Error %d\n", err);
		return err;
	}
	gpio_pin_set(gpiodev, CSB_PIN, 0);

	/*Put the data read from from registers into actual order (see datasheet) */
	//uncompensated pressure value
	datap = (readbuf[1] << 12) | (readbuf[2] << 4) | ((readbuf[3] >> 4) & 0x0F);
	//uncompensated temperature value
	datat = (readbuf[4] << 12) | (readbuf[5] << 4) | ((readbuf[6] >> 4) & 0x0F);
	//uncompensated humidity value
	datah = (readbuf[7] << 8)  | (readbuf[8]);

	//Compensate Pressure, Temperature and Humidity values using given functions	
	bme280_compensate_temp(&bmedata, datat);
	bme280_compensate_press(&bmedata,datap);
	bme280_compensate_humidity(&bmedata, datah);

	//Print the Uncompensated and Compensated values
	printk("\nUncomp-Temp= %d\tComp-Temp= %d \n", datat, bmedata.comp_temp);
	printk("\tUncomp-Pres= %d\tComp-Pres= %d \n", datap, bmedata.comp_press);
	printk("\tUncomp-Hum= %d\tComp-Hum= %d \n", datah, bmedata.comp_humidity);
	return 0;
}

static void bme280_print(const struct device *dev)
{
	printk("Print all characteristics of BME280 sensor\n");

	bme_read_values();
}

static int bme280_open(const struct device *dev)
{
	printk("Open device for BME280 sensor\n");

	return 0;
}

static int bme280_close(const struct device *dev)
{
	printk("Close device for BME280 sensor\n");

	return 0;
}

static int bme280_read_reg(const struct device *dev, uint8_t reg, uint8_t *data, int size)
{
	printk("Read a given register of BME280 sensor\n");

	return 0;
}

static int bme280_write_reg(const struct device *dev,  uint8_t reg, uint8_t value)
{
	printk("Print all characteristics of BME280 sensor\n");

	return 0;
}

static const struct custom_bme280_driver_api custom_bme280_api_funcs = {
	.print      = bme280_print,
	.open       = bme280_open,
	.write_reg  = bme280_write_reg,
	.read_reg   = bme280_read_reg,
	.close      = bme280_close,
};

/* Step 5: Define a device instance using DEVICE_DEFINE */


