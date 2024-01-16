/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* STEP 3.1 - Include the header files for SPI, GPIO and devicetree */
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/devicetree/spi.h>

/* STEP 3.2 - Include display driver and custom header files */
#include <zephyr/sys/byteorder.h>
#include <zephyr/drivers/display.h>
#include "ili_screen_controller.h"

LOG_MODULE_DECLARE(Lesson4_Exercise2, LOG_LEVEL_INF);


/* Structure for ILI9340 controller configuration*/
struct ili_ctrl_config {
	struct spi_dt_spec spi;
	struct gpio_dt_spec cmd_data;
	struct gpio_dt_spec reset;
	uint8_t pixel_format;
	uint16_t rotation;
	uint16_t x_resolution;
	uint16_t y_resolution;
	bool inversion;
	const void *regs;
	int (*regs_init_fn)(const struct device *dev);
};

/* Structure for ILI9340 controller data*/
struct ili_ctrl_data {
	uint8_t bytes_per_pixel;
	enum display_pixel_format pixel_format;
	enum display_orientation orientation;
};

/* Function to fill buffer with RBG color */
void fill_buf_24bit(uint32_t color, uint8_t *buf, size_t buf_size)
{
	for (size_t idx = 0; idx < buf_size; idx += 3) {
		*(buf + idx + 0) = color >> 16;
		*(buf + idx + 1) = color >> 8;
		*(buf + idx + 2) = color >> 0;
	}
}

int spi_ctrl_transmit(const struct device *dev, uint8_t cmd, const void *tx_data, size_t tx_len)
{
	int err;
	const struct ili_ctrl_config *config = dev->config;
	
	/* STEP 4.1 - Declare and configure the transmit buffers with command parameters */
	struct spi_buf tx_buf;
	struct spi_buf_set tx_bufs; 

	tx_buf.buf = &cmd;
	tx_buf.len = 1U;
	tx_bufs.buffers = &tx_buf;
	tx_bufs.count = 1U;

	/* STEP 4.2 - Set GPIO pin for Command and write using spi_write_dt() */
	gpio_pin_set_dt(&config->cmd_data, 1); 
	err = spi_write_dt(&config->spi, &tx_bufs);
	if (err < 0) {
		LOG_ERR("spi_ctrl_transmit: Error on %s", config->spi.bus->name);
		return err;
	}

	/* STEP 4.3 - Transmit the data that follows the command, if any. */
	if (tx_data != NULL) {
		tx_buf.buf = (void *)tx_data;
		tx_buf.len = tx_len;
		gpio_pin_set_dt(&config->cmd_data, 0);
		err = spi_write_dt(&config->spi, &tx_bufs);
		if (err < 0) {
			LOG_ERR("spi_ctrl_transmit: Error on %s", config->spi.bus->name);
			return err;
		}		
	}

	return 0;
}

int ili_ctrl_setmem(const struct device *dev, const uint16_t x, const uint16_t y, \
						   const uint16_t w, const uint16_t h)
{
	int err;
	uint16_t spi_xdata[2];
	uint16_t spi_ydata[2];

	/* Set and transmit column and row starting and ending address */
	spi_xdata[0] = sys_cpu_to_be16(x);
	spi_xdata[1] = sys_cpu_to_be16(x + w - 1U);
	spi_ydata[0] = sys_cpu_to_be16(y);
	spi_ydata[1] = sys_cpu_to_be16(y + h - 1U);

	/* STEP 5 - Call spi_ctrl_transmit() to transmit CASET and PASET commands */
	err = spi_ctrl_transmit(dev, ILI9XXX_CASET, &spi_xdata[0], 4U);	
	if (err < 0) {
		return err;
	}
	err = spi_ctrl_transmit(dev, ILI9XXX_PASET, &spi_ydata[0], 4U);
	if (err < 0) {
		return err;
	}
	return 0;
}

int screen_write(const struct device *dev, const uint16_t x, const uint16_t y, \
			 const struct display_buffer_descriptor *desc, const void *buf)
{
	int err;
	const uint8_t *data_start_addr = (const uint8_t *)buf;

	const struct ili_ctrl_config *config = dev->config;
	struct ili_ctrl_data *data = dev->data;

	struct spi_buf tx_buf;
	struct spi_buf_set tx_bufs;

	uint16_t write_cnt;
	uint16_t nbr_of_writes;
	uint16_t write_h;

	err = ili_ctrl_setmem(dev, x, y, desc->width, desc->height);
	if (err < 0) {
		return err;
	}

	if (desc->pitch > desc->width) {write_h = 1U; nbr_of_writes = desc->height;}
	else {write_h = desc->height; nbr_of_writes = 1U;}

	/* STEP 6 - Call spi_ctrl_transmit() to send the RAMWR command */
	err = spi_ctrl_transmit(dev, ILI9XXX_RAMWR, data_start_addr,
			     desc->width * data->bytes_per_pixel * write_h);
	if (err < 0) {
		return err;
	}

	tx_bufs.buffers = &tx_buf;
	tx_bufs.count = 1;

	data_start_addr += desc->pitch * data->bytes_per_pixel;
	for (write_cnt = 1U; write_cnt < nbr_of_writes; ++write_cnt) {
		tx_buf.buf = (void *)data_start_addr;
		tx_buf.len = desc->width * data->bytes_per_pixel * write_h;

		err = spi_write_dt(&config->spi, &tx_bufs);
		if (err < 0) {
			return err;
		}

		data_start_addr += desc->pitch * data->bytes_per_pixel;
	}

	return 0;
}