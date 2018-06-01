#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <mpsse.h>
#include <endian.h>
#include <unistd.h>

#include "st7567.h"

#define COLS 128

static struct mpsse_context *ctx = NULL;

static void transfer_command(char cmd)
{
	PinLow(ctx, GPIOL0);	// A0
	Start(ctx);
	Write(ctx, &cmd, 1);
	Stop(ctx);
}

static void transfer_data(char cmd)
{
	PinHigh(ctx, GPIOL0);	// A0
	Start(ctx);
	Write(ctx, &cmd, 1);
	Stop(ctx);
}

static void transfer_data_chunk(const char *data, size_t len)
{
	PinHigh(ctx, GPIOL0);	// A0
	Start(ctx);
	Write(ctx, data, len);
	Stop(ctx);
}

static void lcd_set_page(unsigned char page, unsigned int column)
{
	unsigned int lsb = column & 0x0f;
	unsigned int msb = column & 0xf0;
	msb = msb >> 4;
	msb = msb | 0x10;
	page = page | 0xb0;
	transfer_command(page);
	transfer_command(msb);
	transfer_command(lsb);
}

static void clear_ram(void)
{
	unsigned int i = 0;
	for(i = 0; i < 8; i++)
	{
		lcd_set_page(i, 0);
		unsigned int j = 0;
		for(j = 0; j < COLS; j++)
		{
			transfer_data(0);
		}
	}
}

static void pattern_fill(char *buf, size_t len, bool inverted)
{
	for (unsigned j = 0; j < len; ++j) {
		uint8_t data = (j + inverted) % 2 ? 0b10101010 : 0b01010101;
		buf[j] = data;
	}
}

static void set_pattern(bool inverted)
{
	unsigned int i = 0;
	for(i = 0; i < 8; i++)
	{
		lcd_set_page(i, 0);
		unsigned int j = 0;
		for(j = 0; j < COLS; j++)
		{
			uint8_t data = (j + inverted) % 2 ? 0b10101010 : 0b01010101;
			transfer_data(data);
		}
	}
}

static void set_pattern_fast(bool inverted)
{
	const unsigned len = COLS;
	char buf[len];

	pattern_fill(buf, len, inverted);

	for(unsigned i = 0; i < 8; i++)
	{
		lcd_set_page(i, 0);
		transfer_data_chunk(buf, len);
	}
}

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;
	int retval = EXIT_FAILURE;

	if ((ctx = MPSSE(SPI0, 100000, MSB)) != NULL && ctx->open) {
		PinLow(ctx, GPIOL1);	// RST
		usleep(200000);
		PinHigh(ctx, GPIOL1);	// RST
		usleep(50000);
		// from nuttx driver
		transfer_command(ST7567_EXIT_SOFTRST);
		transfer_command(ST7567_BIAS_1_9);
		transfer_command(ST7567_DISPON);
		transfer_command(SSD1305_MAPCOL0);
		transfer_command(ST7567_SETCOMREVERSE);
		transfer_command(ST7567_REG_RES_RR1);

		transfer_command(ST7567_SETEV);
		transfer_command(0x28);	// contrast value

		transfer_command(ST7567_POWERCTRL);
		transfer_command(ST7567_SETSTARTLINE);
		transfer_command(ST7567_SETPAGESTART);
		transfer_command(ST7567_SETCOLH);
		transfer_command(ST7567_SETCOLL);
		transfer_command(ST7567_DISPON);
		transfer_command(ST7567_DISPRAM);
		while (1) {
			clear_ram();
			usleep(500000);
			set_pattern_fast(false);
			usleep(500000);
			set_pattern_fast(true);
			usleep(500000);
		}
		retval = EXIT_SUCCESS;
	} else {
		fprintf(stderr, "Failed to initialize MPSSE: %s\n", ErrorString(ctx));
	}

	Close(ctx);

	return retval;
}
