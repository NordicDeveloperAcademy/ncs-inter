/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include "ili_screen_controller.h"

LOG_MODULE_REGISTER(Lesson4_Ex2, LOG_LEVEL_INF);

#define BKGCOLOR 	0x00U		// Greyscale value for background
#define HLINECOLOR	0xFF0000	// RGB value for line-color (0xRRGGBB)
#define DLINECOLOR  0x0000FF	// RGB value for pixelated diagonal line (0xRRGGBB)
#define BOXCOLOR	0xFFFF00	// RGB value for box-color (0xRRGGBB)
#define DELAY		20			// Delay in ms for visual effect

/* @brief Function to set background color on screen. It will free the buffer before return
   @param screen TFT device pointer
   @param cap Display capabilities of the screen
   @param buf The buffer pointer
   @return ret (success / error code) */
int set_background_color(const struct device *screen, struct display_capabilities cap)
{
	uint8_t *buf;
	size_t buf_size;  
	uint16_t h_step = 1;
	int ret;
	struct display_buffer_descriptor buf_desc;
	
	//Set buffer size for one row and allocate buffer (1 pixel = 3 bytes)
	buf_size = cap.x_resolution;
	buf_size *= 3;
	buf = k_malloc(buf_size);
	if (buf == NULL) {
		LOG_ERR("Memory Allocation Error");
		return -1;
	}

	//Fill the buffer with greyscale and set buffer descriptor
	(void)memset(buf, BKGCOLOR, buf_size);
	buf_desc.buf_size = buf_size;
	buf_desc.width = cap.x_resolution;	
	buf_desc.height = h_step;
	buf_desc.pitch = cap.x_resolution;
	
	//Write background values to screen (row by row) and free buffer
	/* STEP 10 - call the screen_write() for each row and pass buffer */
	for (int idx = 0; idx < cap.y_resolution; idx += h_step) {	
		ret = screen_write(screen, 0, idx, &buf_desc, buf);
		if (ret < 0){
			return ret;
		}
	}
	k_free(buf);
	return 0;
}

/*  @brief Function to draw a diagonal line starting at (x,y)
    @param screen TFT device pointer
    @param x x-coordinate of starting pixel
    @param y y-coordinate of starting pixel
    @param nump number of pixels to draw diagonally
    @return ret (success / error code) */
int draw_diagonal_line(const struct device *screen, uint8_t x, uint8_t y, uint8_t nump)
{
	uint8_t *buf;
	uint8_t buf_size = 1;
	int ret;
	struct display_buffer_descriptor buf_desc;
	buf_size *= 3;

	//Set buffer and descriptor for single pixel
	buf = k_malloc(buf_size);
	if (buf == NULL) {
		LOG_ERR("Memory Allocation Error");
		return -1;
	}
	buf_desc.buf_size = buf_size;
	buf_desc.width = buf_size;
	buf_desc.height = 1;
	buf_desc.pitch = buf_size;

	//Fill the color buffer
	fill_buf_24bit(DLINECOLOR, buf, buf_size);

	//display NUMP number of pixels diagonally
	printk("\n\nDISPLAYING DIAGONAL LINE (pixelated)\n\tStarting pixel (x, y)= %2d, %2d", x, y);
	for(int i=0; i<nump; i++)		
	{		
		/* STEP 11 - Call the screen_write() to display pixel and update coordinates */
		ret = screen_write(screen, x++, y++, &buf_desc, buf);
		if (ret < 0) {
			return ret;
		}
		display_blanking_off(screen);
		k_msleep(DELAY);
	}
	k_free(buf);
	printk("\t\tLast pixel (x, y)= %2d, %2d", x, y);
	return 0;
}

/*  @brief Function to draw multiple straight lines
    @param screen TFT device pointer
    @param x x-coordinate of the first line
    @param y y-coordinate of the first line
    @param dy number of pixels between consecutive lins
    @param w width of the line in pixels
    @param numl total number of lines to draw
    @return ret (success / error code) */
int draw_lines(const struct device *screen, uint8_t x, uint8_t y, uint8_t dy, uint8_t w, uint8_t numl)
{
	uint8_t *buf;
	size_t buf_size;
	int ret;
	struct display_buffer_descriptor buf_desc;

	buf_size = w;
	buf_size *= 3;
	
	//Set buffer and descriptor for a single line		
	buf = k_malloc(buf_size);
	if (buf == NULL) {
		LOG_ERR("Memory Allocation Error");
		return -1;
	}
	buf_desc.buf_size = buf_size;
	buf_desc.width = buf_size;
	buf_desc.height = 1;
	buf_desc.pitch = buf_size;
		
	//Fill the color buffer
	fill_buf_24bit(HLINECOLOR, buf, buf_size);	
	
	//display "numl" number of lines
	printk("\n\nDISPLAYING SERIES OF LINES (1 LINE AT A TIME)\n\tStarting line (x, y)= %2d, %2d", x, y);
	for(int i=0; i<numl; i++)
	{		
		/* STEP 12 - Call the screen_write() and update y-coordinate for new line */
		ret = screen_write(screen, x, y, &buf_desc, buf);
		if (ret < 0) {
			return ret;
		}
		y -= dy;				
		display_blanking_off(screen);
		k_msleep(DELAY);
	}

	k_free(buf);
	printk("\t\tLast line (x, y)= %2d, %2d", x, y);
	return 0;
}

/*  @brief Function to draw a rectuangular box
    @param screen TFT device pointer
    @param x x-coordinate of top left corner
    @param y y-coordinate of top left corner
    @param w width of the box in pixels
    @param h heightof the box in pixels
    @return ret (success / error code) */
int draw_box(const struct device *screen, uint8_t x, uint8_t y, uint8_t w, uint8_t h )
{
	uint8_t *buf;
	size_t buf_size;
	int ret;
	struct display_buffer_descriptor buf_desc;

	//Set buffer and descriptor for rectangular box
	printk("\n\nDISPLAYING A RECTANGULAR BOX BY FILLING WHOLE BUFFER");
	printk("\n\tTop left corner (x, y)= %2d, %2d", x, y);
	buf_size = w * h;
	buf_size *= 3;
	buf = k_malloc(buf_size);
	if (buf == NULL) {
		LOG_ERR("Memory Allocation Error");
		return -1;
	}
	buf_desc.buf_size = buf_size;
	buf_desc.width = w;
	buf_desc.height = h;
	buf_desc.pitch = w;
	
	/* STEP 13 - Oberver that we are sending complete rectangular buffer at once */
	//Fill the color buffer
	fill_buf_24bit(BOXCOLOR, buf, buf_size);
	//Write the buffer to screen at (x,y)
	ret = screen_write(screen, x, y, &buf_desc, buf);
	if (ret < 0) {
		return ret;
	}

	k_free(buf);
	printk("\tBottom right corner (x, y)= %2d, %2d", x+w, y+h);
	display_blanking_off(screen);

	return 0;
}

/*  @brief The main function / Entry point
    It calls defined functions one by one to demonstrate screen usage.
	Those function further use the functions from the custom source file */
int main(void)
{
	//Initialize variables to suitable values (as per screen size)
	size_t x = 0, y = 0, dy = 0;						//for (x,y) coordinates
	size_t w = 80, h = 40;								//for width, height
	const struct device *screen;						//device from DT
	struct display_capabilities cap;					//to get screen capabilities
	uint8_t num_pixels = 100;							//Number of pixels
	uint8_t num_lines = 25;								//Number of lines
	int ret;

	/* STEP 14 - Obtain the screen node from device tree */
	screen = DEVICE_DT_GET(DT_NODELABEL(ili9340));
	if (!device_is_ready(screen)) {
		LOG_ERR("Device %s not found; Aborting", screen->name);
		return 0;
	}

	//Setting orientation 1 (W=320 H=240)
	ret = display_set_orientation(screen, 1);
	if (ret < 0) {
		LOG_ERR("Error %d",ret);
		return 0;
	}
	
	//Setting pixel format 1 (24 bit)
	ret = display_set_pixel_format(screen, 1);
	if (ret < 0) {
		LOG_ERR("Error %d",ret);
		return 0;
	}

	//Obtain and display screen capabilities
	display_get_capabilities(screen, &cap);

	LOG_INF("Display Sample on %s: Orientation=%d,  Pixel-format=%d,   X-res=%d,   Y-res=%d", \
							screen->name, cap.current_orientation, cap.current_pixel_format,  \
							cap.x_resolution, cap.y_resolution);

	//Set the background color
	ret = set_background_color(screen, cap);
	if (ret < 0) {
		LOG_ERR("Error %d",ret);
		return 0;
	}

	//Display a diagonal line (pixel by pixel)
	x = 20;
	y = 20;
	ret = draw_diagonal_line(screen, x, y, num_pixels);
	if (ret < 0) {
		LOG_ERR("Error %d",ret);
		return 0;
	}
	
	//Display a series of lines (1 line at a time)
	x = 100;
	y = 100;
	dy = 3;
	ret = draw_lines(screen, x, y, dy, w, num_lines);
	if (ret < 0) {
		LOG_ERR("Error %d",ret);
		return 0;
	}

	//Display a rectangular box
	x = 30;
	y = 130;
	ret = draw_box(screen, x, y, w, h);
	if (ret < 0) {
		LOG_ERR("Error %d",ret);
		return 0;
	}
	return 0;
}