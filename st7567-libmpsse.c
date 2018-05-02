#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <mpsse.h>
#include <endian.h>
#include <unistd.h>

#include "st7567.h"

static struct mpsse_context *ctx = NULL;

static void transfer_command(char cmd)
{
	usleep(1000);
	PinLow(ctx, GPIOL0);	// A0
	Start(ctx);
	Write(ctx, &cmd, 1);
	Stop(ctx);
}

static void transfer_data(char cmd)
{
	usleep(1000);
	PinHigh(ctx, GPIOL0);	// A0
	Start(ctx);
	Write(ctx, &cmd, 1);
	Stop(ctx);
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
		transfer_command(0x32);
		transfer_command(ST7567_POWERCTRL);
		transfer_command(ST7567_SETSTARTLINE);
		transfer_command(ST7567_SETPAGESTART);
		transfer_command(ST7567_SETCOLH);
		transfer_command(ST7567_SETCOLL);
		transfer_command(ST7567_DISPON);
		while (1) {
			transfer_command(ST7567_DISPRAM);
			sleep(1);
			transfer_command(ST7567_DISPENTIRE);
			sleep(1);
		}
		retval = EXIT_SUCCESS;
	} else {
		fprintf(stderr, "Failed to initialize MPSSE: %s\n", ErrorString(ctx));
	}

	Close(ctx);

	return retval;
}
