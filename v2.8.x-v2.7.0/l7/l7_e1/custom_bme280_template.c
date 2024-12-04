/*
 * Copyright (c) 2019 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/sensor.h>

/* STEP 4 - Define the driver compatible from the custom binding */

LOG_MODULE_REGISTER(custom_bme280, CONFIG_SENSOR_LOG_LEVEL);

#define DELAY_REG 		10
#define DELAY_PARAM		50
#define DELAY_VALUES	1000
#define LED0	13

#define CTRLHUM 		0xF2
#define CTRLMEAS		0xF4
#define CALIB00			0x88
#define CALIB24			0xA1
#define CALIB26			0xE1
#define ID				0xD0
#define PRESSMSB		0xF7
#define PRESSLSB		0xF8
#define PRESSXLSB		0xF9
#define TEMPMSB			0xFA
#define TEMPLSB			0xFB
#define TEMPXLSB		0xFC
#define HUMMSB			0xFD
#define HUMLSB			0xFE
#define DUMMY			0xFF

#define BME280_CHIP_ID  0x60
#define REG_STATUS 		0xF3

#define STATUS_MEASURING 	0x08
#define STATUS_IM_UPDATE 	0x01

#define SPIOP	SPI_WORD_SET(8) | SPI_TRANSFER_MSB

/* STEP 5 - Check if the devicetree contains any devices with the driver compatible */

/* STEP 6.1 - Define data structure to store BME280 data */

/* STEP 6.2 - Define data structure to store sensor configuration data */


int bme280_reg_read(const struct device *dev,
				  uint8_t reg, uint8_t *data, int size)
{
	const struct custom_bme280_config *bus = dev->config;

	uint8_t addr;
	const struct spi_buf tx_spi_buf 	= {.buf = &addr, .len = 1};
	const struct spi_buf_set tx_spi_buf_set 	= {.buffers = &tx_spi_buf, .count = 1};
	struct spi_buf rx_buf[2];
	const struct spi_buf_set rx_spi_buf_set 	= {.buffers = rx_buf, .count = ARRAY_SIZE(rx_buf)};
	
	int i;
	rx_buf[0].buf = NULL;
	rx_buf[0].len = 1;
	rx_buf[1].len = 1;

	for (i = 0; i < size; i++) {
		int err;

		addr = (reg + i) | 0x80;
		rx_buf[1].buf = &data[i];

		err = spi_transceive_dt(&bus->spi, &tx_spi_buf_set, &rx_spi_buf_set);
		if (err) {
			LOG_DBG("spi_transceivedt() failed, err: %d", err);
			return err;
		}
	}

	return 0;

}

int bme280_reg_write(const struct device *dev, uint8_t reg,
				   uint8_t value)
{
	int err;
	const struct custom_bme280_config *bus = dev->config;

	uint8_t tx_buf[] = { reg & 0x7F, value};
	struct spi_buf tx_spi_buf = {.buf = tx_buf, .len = sizeof(tx_buf)};
	struct spi_buf_set tx_spi_buf_set = {.buffers = &tx_spi_buf, .count = 1};
	
	err = spi_write_dt(&bus->spi, &tx_spi_buf_set);
	if (err) {
		LOG_ERR("spi_write_dt() failed, err %d", err);
		return err;
	}
	return 0;
}

/*
 * Compensation code taken from BME280 datasheet, Section 4.2.3
 * "Compensation formula".
 */
void bme280_compensate_temp(struct custom_bme280_data *data, int32_t adc_temp)
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

void bme280_compensate_press(struct custom_bme280_data *data, int32_t adc_press)
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

void bme280_compensate_humidity(struct custom_bme280_data *data, int32_t adc_humidity)
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

int bme280_wait_until_ready(const struct device *dev)
{
	uint8_t status = 0;
	int ret;

	/* Wait for NVM to copy and and measurement to be completed */
	do {
		k_sleep(K_MSEC(3));
		ret = bme280_reg_read(dev, REG_STATUS, &status, 1);
		if (ret < 0) {
			return ret;
		}
	} while (status & (STATUS_MEASURING | STATUS_IM_UPDATE));

	return 0;
}

static int custom_bme280_sample_fetch(const struct device *dev,
				      enum sensor_channel chan)
{	
	/* STEP 7.1 - Populate the custom_bme280_sample_fetch() function */



	return 0;
}

static int custom_bme280_channel_get(const struct device *dev,
				     enum sensor_channel chan,
				     struct sensor_value *val)
{
	/* STEP 7.2 - Populate the custom_bme280_channel_get() function */


	return 0;
}

/* STEP 7.3 - Define the sensor driver API */


int bme280_read_compensation(const struct device *dev)
{
    struct custom_bme280_data *data = dev->data;
	uint16_t buf[12];
	uint8_t hbuf[7];
	int err = 0;

	err = bme280_reg_read(dev, CALIB00,
			      (uint8_t *)buf, sizeof(buf));

	if (err < 0) {
		LOG_DBG("dig_T1-T3, dig_P1-P9 read failed: %d", err);
		return err;
	}

	data->dig_t1 = sys_le16_to_cpu(buf[0]);
	data->dig_t2 = sys_le16_to_cpu(buf[1]);
	data->dig_t3 = sys_le16_to_cpu(buf[2]);

	data->dig_p1 = sys_le16_to_cpu(buf[3]);
	data->dig_p2 = sys_le16_to_cpu(buf[4]);
	data->dig_p3 = sys_le16_to_cpu(buf[5]);
	data->dig_p4 = sys_le16_to_cpu(buf[6]);
	data->dig_p5 = sys_le16_to_cpu(buf[7]);
	data->dig_p6 = sys_le16_to_cpu(buf[8]);
	data->dig_p7 = sys_le16_to_cpu(buf[9]);
	data->dig_p8 = sys_le16_to_cpu(buf[10]);
	data->dig_p9 = sys_le16_to_cpu(buf[11]);

	err = bme280_reg_read(dev, CALIB24,
			      &data->dig_h1, 1);
	if (err < 0) {
		LOG_DBG("dig_H1 read failed: %d", err);
		return err;
	}

	err = bme280_reg_read(dev, CALIB26, hbuf, 7);
	if (err < 0) {
		LOG_DBG("dig_H2-H6 read failed: %d", err);
		return err;
	}

	data->dig_h2 = (hbuf[1] << 8) | hbuf[0];
	data->dig_h3 = hbuf[2];
	data->dig_h4 = (hbuf[3] << 4) | (hbuf[4] & 0x0F);
	data->dig_h5 = ((hbuf[4] >> 4) & 0x0F) | (hbuf[5] << 4);
	data->dig_h6 = hbuf[6];


	return 0;
}

static int custom_bme280_init(const struct device *dev)
{
	struct custom_bme280_data *data = dev->data;
	int err;

	err = bme280_reg_read(dev, ID, &data->chip_id, 1);
	if (err < 0) {
		LOG_DBG("ID read failed: %d", err);
		return err;
	}

	if (data->chip_id == BME280_CHIP_ID) {
		LOG_DBG("ID OK");
	} else {
		LOG_DBG("Bad chip id 0x%x", data->chip_id);
		return -ENOTSUP;
	}

	err = bme280_wait_until_ready(dev);
	if (err < 0) {
		return err;
	}

	err = bme280_read_compensation(dev);
	if (err < 0) {
		return err;
	}
	//Numbers  (0x04) taken from previous sample
	err = bme280_reg_write(dev, CTRLHUM, 0x04);
	if (err < 0) {
		LOG_DBG("CTRL_HUM write failed: %d", err);
		return err;
	}
	
	//Numbers (0x93) taken from previous sample
	err = bme280_reg_write(dev, CTRLMEAS, 0x93);
	if (err < 0) {
		LOG_DBG("CTRL_MEAS write failed: %d", err);
		return err;
	}

	/* Wait for the sensor to be ready */
	k_sleep(K_MSEC(1));

	LOG_DBG("\"%s\" OK", dev->name);
	return 0;
}

/* STEP 8 - Define a macro for the device driver instance */


/* STEP 9 - Create the struct device for every status "okay" node in the devicetree */