/*
 * Copyright (c) 2017 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 * Copyright (c) 2023 Nordic Semiconductor ASA
 * Copyright (c) 2020 Teslabs Engineering S.L.
 * Copyright (c) 2021 Krivorot Oleg <krivorot.oleg@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ILI_SCREEN_CONTROLLER_H_
#define ILI_SCREEN_CONTROLLER_H_

/* Commands/registers. */
#define ILI9XXX_SWRESET 0x01
#define ILI9XXX_SLPOUT 0x11
#define ILI9XXX_DINVON 0x21
#define ILI9XXX_GAMSET 0x26
#define ILI9XXX_DISPOFF 0x28
#define ILI9XXX_DISPON 0x29
#define ILI9XXX_CASET 0x2a
#define ILI9XXX_PASET 0x2b
#define ILI9XXX_RAMWR 0x2c
#define ILI9XXX_MADCTL 0x36
#define ILI9XXX_PIXSET 0x3A

/* MADCTL register fields. */
#define ILI9XXX_MADCTL_MY BIT(7U)
#define ILI9XXX_MADCTL_MX BIT(6U)
#define ILI9XXX_MADCTL_MV BIT(5U)
#define ILI9XXX_MADCTL_ML BIT(4U)
#define ILI9XXX_MADCTL_BGR BIT(3U)
#define ILI9XXX_MADCTL_MH BIT(2U)

/* PIXSET register fields. */
#define ILI9XXX_PIXSET_RGB_18_BIT 0x60
#define ILI9XXX_PIXSET_RGB_16_BIT 0x50
#define ILI9XXX_PIXSET_MCU_18_BIT 0x06
#define ILI9XXX_PIXSET_MCU_16_BIT 0x05

/** Command/data GPIO level for Data/Command. */
#define ILI9XXX_DATA 0U
#define ILI9XXX_CMD 1U

/** Sleep out time (ms), ref. 8.2.12 of ILI9XXX manual. */
#define ILI9XXX_SLEEP_OUT_TIME 120

/** Reset pulse time (ms), ref 15.4 of ILI9XXX manual. */
#define ILI9XXX_RESET_PULSE_TIME 1

/** Reset wait time (ms), ref 15.4 of ILI9XXX manual. */
#define ILI9XXX_RESET_WAIT_TIME 5

void fill_buf_24bit(uint32_t color, uint8_t *buf, size_t buf_size);

int spi_ctrl_transmit(const struct device *dev, uint8_t cmd, const void *tx_data, size_t tx_len);

int ili_ctrl_setmem(const struct device *dev, const uint16_t x, const uint16_t y, \
						   const uint16_t w, const uint16_t h);

int screen_write(const struct device *dev, const uint16_t x, const uint16_t y, \
			 const struct display_buffer_descriptor *desc, const void *buf);

#endif /* ILI_SCREEN_CONTROLLER_H_ */