/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* STEP 1.2 - Include the header files for SPI, GPIO and devicetree */
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

LOG_MODULE_REGISTER(Lesson4_Exercise1, LOG_LEVEL_INF);

#define READ_DELAY 		100
#define LEDDELAY 		3000

// Define the chip-select bar (CSB) pin number for the board
#ifdef CONFIG_BOARD_NRF7002DK_NRF5340_CPUAPP
	#define CSB_PIN 25
#endif
#ifdef CONFIG_BOARD_NRF5340DK_NRF5340_CPUAPP
	#define CSB_PIN 25
#endif
#ifdef CONFIG_BOARD_NRF52840DK_NRF52840
	#define CSB_PIN 30
#endif
#ifdef CONFIG_BOARD_NRF52DK_NRF52832
	#define CSB_PIN 30
#endif
#ifdef CONFIG_BOARD_NRF52833DK_NRF52833
	#define CSB_PIN 30
#endif
#ifdef CONFIG_BOARD_NRF9160DK_NRF9160
	#define CSB_PIN 18
#endif

// Define LED0 pin number
#define LED0	13

/* STEP 3 - Obtain the SPI controller node and GPIO node for CS pin */
static const struct device * spidev = DEVICE_DT_GET(DT_NODELABEL(spi1));
const struct device * gpiodev = DEVICE_DT_GET(DT_NODELABEL(gpio0));

/* STEP 4 - Define and initialize an spi_config structure */
const struct spi_config bme_spi_config = {
	.frequency = 125000U,
	.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
	.slave = 0,
};

// Data structure to store BME280 data
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
}bmedata;

/// @brief The function to read-out a single BME280 register
/// @param reg is the address of the register
/// @param data is the pointer to store read data
/// @param size is the size of data
/// @return 0 or err code
static int bme_read_reg(uint8_t reg, uint8_t *data, int size)
{
	int err;

	/* STEP 5.1 - Set the transmit and receive buffers */
	uint8_t tx_buffer = reg;
	struct spi_buf tx_spi_buf = {.buf = (void *)&tx_buffer, .len = 1};
	struct spi_buf_set tx_spi_buf_set = {.buffers = &tx_spi_buf, .count = 1};
	struct spi_buf rx_spi_bufs = {.buf = data, .len = size};
	struct spi_buf_set rx_spi_buffer_set = {.buffers = &rx_spi_bufs, .count = 1};

	/* STEP 5.2 - Enable the sensor by activating the chip-select signal */
	gpio_pin_set(gpiodev, CSB_PIN, 1);

	/* STEP 5.3 - Write the transmit buffers to the sensor */
	err = spi_write(spidev, &bme_spi_config, &tx_spi_buf_set);
	if (err < 0) {
		LOG_ERR("spi_write() failed, err: %d", err);
		return err;
	}

	/* STEP 5.4 - Read the data from the sensor */
	err = spi_read(spidev, &bme_spi_config, &rx_spi_buffer_set);
	if (err < 0) {
		LOG_ERR("spi_read() failed, err: %d", err);
		return err;
	}

	/* STEP 5.5 - Put the device into inactive mode by disabling the cnip-select signal */
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

	/* STEP 6.1 - [] */
	/* Bit7 is 0 for the write command */
	uint8_t tx_buf[] = {(reg & 0x7F), value};

	/* STEP 6.2 - Write the data-byte to the sensor register */
	struct spi_buf tx_spi_buf = {.buf = tx_buf, .len = sizeof(tx_buf)};
	struct spi_buf_set tx_spi_buf_set = {.buffers = &tx_spi_buf, .count = 1};
	gpio_pin_set(gpiodev, CSB_PIN, 1);
	err = spi_write(spidev, &bme_spi_config, &tx_spi_buf_set);
	if (err < 0) {
		LOG_ERR("spi_write() failed, err %d", err);
		return err;
	}
	gpio_pin_set(gpiodev, CSB_PIN, 0);

	return 0;
}

/// @brief The function to read calibration registers and 
///        set the values in bme_data struct so that 
///        calibrating functiions could use these values
void bme_calibrationdata_read(void){
	uint8_t values[2];
	uint8_t regaddr;

	/* STEP 7 - Go through the following code. We are using bme_read_reg()
	to read the required number of bytes from respective register locations
	and setting the data in proper order to form the compensation parameter values */
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

	LOG_INF("Reading the parameters from sensor for temperature, pressure, and humidity");
	LOG_INF("T1: %d\tT2: %d\tT3: %d", bmedata.dig_t1, bmedata.dig_t2, bmedata.dig_t3);
	LOG_INF("P1: %d\tP2: %d\tP3: %d", bmedata.dig_p1, bmedata.dig_p2, bmedata.dig_p3);
	LOG_INF("P4: %d\tP5: %d\tP5: %d", bmedata.dig_p4, bmedata.dig_p5, bmedata.dig_p6);
	LOG_INF("P7: %d\tP8: %d\tP9: %d", bmedata.dig_p7, bmedata.dig_p8, bmedata.dig_p9);
	LOG_INF("H1: %d\tH2: %d\tH3: %d", bmedata.dig_h1, bmedata.dig_h2, bmedata.dig_h3);
	LOG_INF("H4: %d\tH5: %d\tH6: %d\n", bmedata.dig_h4, bmedata.dig_h5, bmedata.dig_h6);
}

/// @brief The function to read and print different BME280 registers
/// 		It sets register addresses as per the data-sheet
///			and then calls the read_bme_reg() function to read
///			registers one by one and display the contents
int bme_print_regs(void)
{
	uint8_t buf;
	uint8_t size = 1;
	uint8_t data;
	int err;

	/* STEP 8 - Go through the following code and see how we are using
	bme_read_reg() to read and print different registers (1-byte) */

	// Register addresses to read from (see the data sheet)
	uint8_t reg_id = 0xD0;
	uint8_t regs_morecalib[16];
	uint8_t regs_more[12];
	
	// Set the register addresses
	regs_morecalib[0] = 0xE1;
	for (uint8_t i=0; i<15; i++)
		regs_morecalib[i+1] = regs_morecalib[i] + 1;
	
	regs_more[0] = 0xF2;
	for (uint8_t i=0; i<11; i++)
		regs_more[i+1] = regs_more[i] + 1;

	// Read 1 byte data from each register and print
	LOG_INF("Reading all BME280 registers (one by one with delay = %d ms):", READ_DELAY);	
	err = bme_read_reg(reg_id, &buf, size);
	if (err < 0)
	{
		LOG_INF("Error in bme_read_reg(), err: %d", err);
		return err;
	}
	data = buf;
	bmedata.chip_id = data;
	LOG_INF("\tReg[0x%02x]  =  0x%02x", reg_id, data);
	k_msleep(READ_DELAY);
	
	// Reading from more calibration registers
	for (uint8_t i = 0; i < sizeof(regs_morecalib); i++)
	{
		err = bme_read_reg(regs_morecalib[i], &buf, size);
		if (err < 0)
		{
			LOG_INF("Error in bme_read_reg(), err: %d", err);
			return err;
		}
		data = buf;
		LOG_INF("\tReg[0x%02x]  =  0x%02x", regs_morecalib[i], data);
		k_msleep(READ_DELAY);
	}

	// Reading from more registers
	for (uint8_t i=0; i<sizeof(regs_more); i++)
	{
		err = bme_read_reg(regs_more[i], &buf, size);
		if (err < 0)
		{
			LOG_INF("Error in bme_read_reg(), err: %d", err);
			return err;
		}
		data = buf;
		LOG_INF("\tReg[0x%02x]  =  0x%02x", regs_more[i], data);
		k_msleep(READ_DELAY);
	}

	return 0;
}

/* STEP 9 - Go through the compensation functions and 
  note that they use the compensation parameters from 
  the bme280_data structure and then store back the compensated value */
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
///        temperature, pressure, and humidity values from
///        the BME280 device
int bme_read_values(void)
{

	int err;

	int32_t datap = 0, datat = 0, datah = 0;
	float pressure = 0.0, temperature = 0.0, humidity = 0.0;

	/* STEP 10.1 - Store register addresses to do burst read */
	uint8_t regs[] = {0xF7, 0xF8, 0xF9, \
					  0xFA, 0xFB, 0xFC, \
					  0xFD, 0xFE, 0xFF};
	uint8_t readbuf[sizeof(regs)];

	/* STEP 10.2 - Set the transmit and receive buffers */
	struct spi_buf tx_spi_buf = {.buf = (void *)&regs, .len = sizeof(regs)};
	struct spi_buf_set tx_spi_buf_set = {.buffers = &tx_spi_buf, .count = 1};
	struct spi_buf rx_spi_bufs = {.buf = readbuf, .len = sizeof(regs)};
	struct spi_buf_set rx_spi_buffer_set = {.buffers = &rx_spi_bufs, .count = 1};

	/* STEP 10.3 - Use spi_transceive() to transmit and receive at the same time */
	gpio_pin_set(gpiodev, CSB_PIN, 1);
	err = spi_transceive(spidev, &bme_spi_config, &tx_spi_buf_set, &rx_spi_buffer_set);	
	if (err < 0) {
		LOG_ERR("spi_transceive() failed, err: %d", err);
		return err;
	}
	gpio_pin_set(gpiodev, CSB_PIN, 0);

	/* Put the data read from registers into actual order (see datasheet) */
	// Uncompensated pressure value
	datap = (readbuf[1] << 12) | (readbuf[2] << 4) | ((readbuf[3] >> 4) & 0x0F);
	// Uncompensated temperature value
	datat = (readbuf[4] << 12) | (readbuf[5] << 4) | ((readbuf[6] >> 4) & 0x0F);
	// Uncompensated humidity value
	datah = (readbuf[7] << 8)  | (readbuf[8]);

	// Compensate pressure, temperature and humidity values using given functions	
	bme280_compensate_temp(&bmedata, datat);
	bme280_compensate_press(&bmedata,datap);
	bme280_compensate_humidity(&bmedata, datah);

	// Convert to proper format as per data sheet
	temperature = (float)bmedata.comp_temp/100.0;
	pressure 	= (float)bmedata.comp_press/256.0;
	humidity 	= (float)bmedata.comp_humidity/1024.0;
	
	// Print the uncompensated and compensated values
	LOG_INF("Temp \t uncomp = %d C \t comp = %f C", datat, temperature);
	LOG_INF("Pressure \t uncomp = %d Pa \t comp = %f Pa", datap, pressure);
	LOG_INF("Humidity \t uncomp = %d RH \t comp = %f%% RH", datah, humidity);

	return 0;
}

/// @brief  The main entry function
///			First Calibration Data is read, 
///			next, sampling parameters are written
///			and then data is read in while(1)
int main(void)
{
	int err;
	
	/* STEP 11.1 - Check if SPI and GPIO devices are ready */
	err = device_is_ready(spidev);
	if (!err) {
		LOG_ERR("Error: SPI device is not ready, err: %d", err);
		return 0;
	}

	err = device_is_ready(gpiodev);
	if (!err) {
		LOG_ERR("Error: GPIO device is not ready, err: %d", err);
		return 0;
	}

	/* STEP 11.2 - Configure the chip select pin and LED0 */
	gpio_pin_configure(gpiodev, CSB_PIN, GPIO_OUTPUT | GPIO_ACTIVE_LOW);
	gpio_pin_configure(gpiodev, LED0, GPIO_OUTPUT_ACTIVE);

	/* STEP 11.3 - Read calibration data */
	bme_calibrationdata_read();

	/* STEP 11.4 - Write sampling parameters and read and print the registers */
	bme_write_reg(0xF2, 0x04);
	bme_write_reg(0xF4, 0x93);

	bme_print_regs();

	// Continuously read data values and toggle led	
	LOG_INF("Continuously read sensor samples, compensate, and display (delay = %d ms) ", LEDDELAY);
	while(1){
		/* STEP 11.5 - Continously read sensor samples and toggle LED */
		bme_read_values();
		gpio_pin_toggle(gpiodev, LED0);
		k_msleep(LEDDELAY);
	}

	return 0;
}
