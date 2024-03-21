/*
 * Copyright (c) 2019 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "custom_bme280_driver_spi.h"
#include <zephyr/types.h>
#include <zephyr/sys/printk.h>
#include <ncs_version.h>
#if NCS_VERSION_NUMBER >= 0x20600
#include <zephyr/internal/syscall_handler.h>
#else
#include <zephyr/syscall_handler.h>
#endif

#include <zephyr/drivers/spi.h>

#define DELAY_REG 		10
#define DELAY_PARAM		50
#define DELAY_VALUES	1000
#define LED0	13

#define CTRLHUM 		0xF2
#define CTRLMEAS		0xF4
#define CALIB00			0x88
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

#define SPIOP	SPI_WORD_SET(8) | SPI_TRANSFER_MSB

#if DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 0
#warning "Custom BME280 driver enabled without any devices"
#endif

/* Data structure to store BME280 data */
struct bme280_data {
	/* Compensation parameters */
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

	/* Compensated values */
	int32_t comp_temp;
	uint32_t comp_press;
	uint32_t comp_humidity;

	/* Carryover between temperature and pressure/humidity compensation */
	int32_t t_fine;

	uint8_t chip_id;
} bmedata;

struct custom_bme280_config {
	struct spi_dt_spec spi;
};

/*
 @brief The function to read-out a single BME280 register
 @param reg is the address of the register
 @param data is the pointer to store read data
 @param size is the size of data
 @return 0 or err code
*/
static int bme_read_reg(const struct device *dev, uint8_t reg, uint8_t *data, int size)
{
	int err;

	uint8_t tx_buffer = reg;

	/* Set the transmit and receive buffers */
	struct spi_buf tx_spi_buf = {.buf = (void *)&tx_buffer, .len = 1};	
	struct spi_buf_set tx_spi_buf_set = {.buffers = &tx_spi_buf, .count = 1};
	struct spi_buf rx_spi_bufs = {.buf = data, .len = size};
	struct spi_buf_set rx_spi_buf_set = {.buffers = &rx_spi_bufs, .count = 1};

	const struct custom_bme280_config *bme280_config = dev->config;
	err = spi_transceive_dt(&bme280_config->spi, &tx_spi_buf_set, &rx_spi_buf_set);
	if (err < 0) {
		printk("spi_transceive_dt() failed, err: %d", err);
		return err;
	}

	return 0;
}

/// @brief The function to write a value to the BME280 register
/// @param reg is the address of the register
/// @param values is the data to write
/// @return 0 or err code
static int bme_write_reg(const struct device *dev, uint8_t reg, uint8_t value)
{
	int err;

	const struct custom_bme280_config *bme280_config = dev->config;

	/* Bit7 is 0 for the write command */
	uint8_t tx_buf[] = {(reg & 0x7F), value};

	struct spi_buf tx_spi_buf = {.buf = tx_buf, .len = sizeof(tx_buf)};
	struct spi_buf_set tx_spi_buf_set = {.buffers = &tx_spi_buf, .count = 1};
	err = spi_write_dt(&bme280_config->spi, &tx_spi_buf_set);
	if (err < 0) {
		printk("spi_write_dt() failed, err %d", err);
		return err;
	}

	return 0;
}

/* @brief The function to read calibration registers and 
        set the values in bme_data struct so that 
        calibrating functiions could use these values 
*/
void bme_calibrationdata(const struct device *dev){
	/* Set data size of 3 as the first byte is dummy using bme_read_reg() */
	uint8_t values[3];
	uint8_t size = 3;	
	
	uint8_t regaddr;
	
	/* Reading required number of bytes from respective register location
	and setting data in proper order to form compensation parameter values*/

	regaddr = CALIB00;
	bme_read_reg(dev, regaddr, values, size);
	bmedata.dig_t1 = ((uint16_t)values[2])<<8 | values[1];
	k_msleep(DELAY_PARAM);

	regaddr += 2;
	bme_read_reg(dev, regaddr, values, size);
	bmedata.dig_t2 = ((uint16_t)values[2])<<8 | values[1];
	k_msleep(DELAY_PARAM);
	
	regaddr += 2;
	bme_read_reg(dev, regaddr, values, size);
	bmedata.dig_t3 = ((uint16_t)values[2])<<8 | values[1];
	k_msleep(DELAY_PARAM);
	
	regaddr += 2;
	bme_read_reg(dev, regaddr, values, size);
	bmedata.dig_p1 = ((uint16_t)values[2])<<8 | values[1];
	k_msleep(DELAY_PARAM);

	regaddr += 2;
	bme_read_reg(dev, regaddr, values, size);
	bmedata.dig_p2 = ((uint16_t)values[2])<<8 | values[1];
	k_msleep(DELAY_PARAM);
	
	regaddr += 2;
	bme_read_reg(dev, regaddr, values, size);
	bmedata.dig_p3 = ((uint16_t)values[2])<<8 | values[1];
	k_msleep(DELAY_PARAM);
	
	regaddr += 2;
	bme_read_reg(dev, regaddr, values, size);
	bmedata.dig_p4 = ((uint16_t)values[2])<<8 | values[1];
	k_msleep(DELAY_PARAM);
	
	regaddr += 2;
	bme_read_reg(dev, regaddr, values, size);
	bmedata.dig_p5 = ((uint16_t)values[2])<<8 | values[1];
	k_msleep(DELAY_PARAM);
	
	regaddr += 2;
	bme_read_reg(dev, regaddr, values, size);
	bmedata.dig_p6 = ((uint16_t)values[2])<<8 | values[1];
	k_msleep(DELAY_PARAM);
	
	regaddr += 2;
	bme_read_reg(dev, regaddr, values, size);
	bmedata.dig_p7 = ((uint16_t)values[2])<<8 | values[1];
	k_msleep(DELAY_PARAM);
	
	regaddr += 2;
	bme_read_reg(dev, regaddr, values, size);
	bmedata.dig_p8 = ((uint16_t)values[2])<<8 | values[1];
	k_msleep(DELAY_PARAM);
	
	regaddr += 2;
	bme_read_reg(dev, regaddr, values, size);
	bmedata.dig_p9 = ((uint16_t)values[2])<<8 | values[1];
	k_msleep(DELAY_PARAM);
	
	regaddr += 3; size=2; /* only read one byte for H1 (see datasheet) */
	bme_read_reg(dev, regaddr, values, size);
	bmedata.dig_h1 = (uint8_t)values[1];
	k_msleep(DELAY_PARAM);
	
	regaddr += 64; size=3; /* read two bytes for H2 */
	bme_read_reg(dev, regaddr, values, size);
	bmedata.dig_h2 = ((uint16_t)values[2])<<8 | values[1];
	k_msleep(DELAY_PARAM);
	
	regaddr += 2; size=2; /* only read one byte for H3 */
	bme_read_reg(dev, regaddr, values, size);
	bmedata.dig_h3 = (uint8_t)values[1];
	k_msleep(DELAY_PARAM);
	
	regaddr += 1; size=3; /* read two bytes for H4 */
	bme_read_reg(dev, regaddr, values, size);
	bmedata.dig_h4 = ((uint16_t)values[1])<<4 | (values[2] & 0x0F);
	k_msleep(DELAY_PARAM);
	
	regaddr += 1;
	bme_read_reg(dev, regaddr, values, size);
	bmedata.dig_h5 = ((uint16_t)values[2])<<4 | ((values[1] >> 4) & 0x0F);
	k_msleep(DELAY_PARAM);
	
	regaddr += 2; size=2; /* only read one byte for H6 */
	bme_read_reg(dev, regaddr, values, 2);
	bmedata.dig_h6 = (uint8_t)values[1];
	k_msleep(DELAY_PARAM);

	printk("\nBME280 calibration data from sensor BME280 read and saved \n");
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

	int err;

	const struct custom_bme280_config *bme280_config = dev->config;
	err = spi_is_ready_dt(&bme280_config->spi);
	if (!err) {
		printk("Error: SPI device is not ready, err: %d", err);
		return 0;
	}
	
	bme_calibrationdata(dev);
	bme_write_reg(dev, CTRLHUM, 0x04);
	bme_write_reg(dev, CTRLMEAS, 0x93);	

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

	/* Avoid exception caused by division by zero */
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
int bme_read_values(const struct device *dev)
{
	int err;

	const struct custom_bme280_config *bme280_config = dev->config;
	
	int32_t datap = 0, datat = 0, datah = 0;
	float pressure = 0.0, temperature = 0.0, humidity = 0.0;

	/* Store register addresses to do burst read */
	uint8_t regs[] = {PRESSMSB, PRESSLSB, PRESSXLSB, \
					  TEMPMSB, TEMPLSB, TEMPXLSB, \
					  HUMMSB, HUMLSB, DUMMY};	//0xFF is dummy reg
	uint8_t readbuf[sizeof(regs)];

	/* Set the transmit and receive buffers */
	struct spi_buf 		tx_spi_buf 			= {.buf = (void *)&regs, .len = sizeof(regs)};
	struct spi_buf_set 	tx_spi_buf_set		= {.buffers = &tx_spi_buf, .count = 1};
	struct spi_buf 		rx_spi_bufs			= {.buf = readbuf, .len = sizeof(regs)};
	struct spi_buf_set 	rx_spi_buf_set	= {.buffers = &rx_spi_bufs, .count = 1};
	
	/* Use spi_transceive() to transmit and receive at the same time */
	err = spi_transceive_dt(&bme280_config->spi, &tx_spi_buf_set, &rx_spi_buf_set);
	if (err < 0) {
		printk("spi_transceive_dt() failed, err: %d", err);
		return err;
	}

	/* Put the data read from registers into actual order (see datasheet) */
	/* Uncompensated pressure value */
	datap = (readbuf[1] << 12) | (readbuf[2] << 4) | ((readbuf[3] >> 4) & 0x0F);
	/* Uncompensated temperature value */
	datat = (readbuf[4] << 12) | (readbuf[5] << 4) | ((readbuf[6] >> 4) & 0x0F);
	/* Uncompensated humidity value */
	datah = (readbuf[7] << 8)  | (readbuf[8]);

	/* Compensate values using given functions */
	bme280_compensate_press(&bmedata,datap);
	bme280_compensate_temp(&bmedata, datat);
	bme280_compensate_humidity(&bmedata, datah);

	/* Convert to proper format as per data sheet */
	pressure 	= (float)bmedata.comp_press/256.0;
	temperature = (float)bmedata.comp_temp/100.0;
	humidity 	= (float)bmedata.comp_humidity/1024.0;
	
	/* Print the uncompensated and compensated values */
	printk("\tTemperature: = %d C\n",  (int)temperature);
	printk("\tPressure   : = %d Pa\n", (int)pressure);	
	printk("\tHumidity   : = %d RH\n", (int)humidity);

	return 0;
}

static void bme280_print(const struct device *dev)
{
	printk("Print all characteristics of BME280 sensor\n");

	bme_read_values(dev);
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

	return bme_read_reg(dev, reg, data, size);
}

static int bme280_write_reg(const struct device *dev,  uint8_t reg, uint8_t value)
{
	printk("Write to a given register of BME280 sensor\n");

	return bme_write_reg(dev, reg, value);
}

static const struct custom_bme280_driver_api custom_bme280_api_funcs = {
	.print      = bme280_print,
	.open       = bme280_open,
	.write_reg  = bme280_write_reg,
	.read_reg   = bme280_read_reg,
	.close      = bme280_close,
};

/* Initializes a struct bme280_config for an instance on a SPI bus. */
#define BME280_CONFIG_SPI(inst)				\
	{						\
		.spi = SPI_DT_SPEC_INST_GET(inst, SPIOP, 0),	\
	}

/* STEP 5.1 - Define a device driver instance */


/* STEP 5.2 - Create the struct device for every status "okay" node in the devicetree */

