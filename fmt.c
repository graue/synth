#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synth.h"
#define MT_IMPLEMENT
#include "mt.h"

/* fmt: convert floats on stdin to their final format */

static int monodst = 0;
static int dodither = 0;
static void conv_u8(void);
static void conv_s8(void);
static void conv_16(void);
static void conv_24(void);
static void conv_32(void);

int main(int argc, char *argv[])
{
	int fmt = 2;
	int i;

	/* read options */
	for (i = 1; i < argc; i++)
	{
		     if (!strcmp(argv[i], "-u8")) fmt = 0; /* 8-bit unsigned */
		else if (!strcmp(argv[i], "-s8")) fmt = 1; /* 8-bit signed */
		else if (!strcmp(argv[i], "-16")) fmt = 2; /* 16-bit signed */
		else if (!strcmp(argv[i], "-24")) fmt = 3; /* 24-bit signed */
		else if (!strcmp(argv[i], "-32")) fmt = 4; /* 32-bit signed */
		else if (!strcmp(argv[i], "-mono")) monodst = 1;
		else if (!strcmp(argv[i], "-dither")) dodither = 1;
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -u8, -s8, -16, -24, -32, "
				"-mono, -dither\n");
			exit(0);
		}
	}

	SET_BINARY_MODE
	if (fmt == 0)
		conv_u8();
	else if (fmt == 1)
		conv_s8();
	else if (fmt == 2)
		conv_16();
	else if (fmt == 3)
		conv_24();
	else if (fmt == 4)
		conv_32();

	return 0;
}

/* returns 1 on success */
static int nextsample(double *dst)
{
	float f[2];

	if (monodst)
	{
		if (fread(f, sizeof f[0], 2, stdin) < 2)
			return 0;
		*dst = (f[0] + f[1]) / 2.0;
	}
	else
	{
		if (fread(&f[0], sizeof f[0], 1, stdin) < 1)
			return 0;
		*dst = f[0];
	}

	return 1;
}

/* returns random value in [-1, 1) */
double dithernoise(void)
{
	return mt_frand() * 2.0 - 1.0;
}

static void conv_u8(void)
{
	double f;
	unsigned char s;

	while (nextsample(&f))
	{
		/* Expand range from [-1, 1] to about [-128, 127]. */
		f *= 128.0;

		/* Move range from -128 .. 127 to 0 .. 255. */
		f += 128.0;

		/* Dither. */
		if (dodither)
			f += dithernoise();

		/* Clip. */
		if (f < 0.0)
			f = 0.0;
		else if (f > 255.0)
			f = 255.0;

		s = (unsigned char)f;
		putchar((char)s);
	}
}

static void conv_s8(void)
{
	double f;
	signed char s;

	while (nextsample(&f))
	{
		/* Expand range from [-1, 1] to about [-128, 127]. */
		f *= 128.0;

		/* Dither. */
		if (dodither)
			f += dithernoise();

		/* Clip. */
		if (f < -128.0)
			f = -128.0;
		else if (f > 127.0)
			f = 127.0;

		s = (signed char)f;
		putchar((char)s);
	}
}

static void conv_16(void)
{
	double f;
	short s;

	while (nextsample(&f))
	{
		/* Expand range from [-1, 1] to about [-32768, 32767]. */
		f *= 32768.0;

		/* Dither. */
		if (dodither)
			f += dithernoise();

		/* Clip. */
		if (f < -32768.0)
			f = -32768.0;
		else if (f > 32767.0)
			f = 32767.0;

		s = (short)f;
		fwrite(&s, sizeof s, 1, stdout);
	}
}

static void conv_24(void)
{
	double f;
	int s;
	unsigned char *sw;

	sw = (unsigned char *)&s;

	while (nextsample(&f))
	{
		/* Expand from [-1, 1] to about [-8388608, 8388607]. */
		f *= 8388608.0;

		/* Dither. */
		if (dodither)
			f += dithernoise();

		/* Clip. */
		if (f > 8388607.0) f = 8388607.0;
		else if (f < -8388608.0) f = -8388608.f;

		s = (int)f;
#if WORDS_BIGENDIAN
		if (sw[0] == 0xff) putc(sw[1] | 0x80, stdout);
		else putc(sw[1], stdout);
		putc(sw[2], stdout);
		putc(sw[3], stdout);
#else
		putc(sw[0], stdout);
		putc(sw[1], stdout);
		if (sw[3] == 0xff) putc(sw[2] | 0x80, stdout);
		else putc(sw[2], stdout);
#endif
	}
}

static void conv_32(void)
{
	double f;
	int s;

	while (nextsample(&f))
	{
		/* Expand from [-1, 1] to about [-2147483648, 2147483647]. */
		f *= 2147483648.0;

		/* Dither. */
		if (dodither)
			f += dithernoise();

		/* Clip. */
		if (f > 2147483647.0) f = 2147483647.0;
		else if (f < -2147483648.0) f = -2147483648.f;

		s = (int)f;
		fwrite(&s, sizeof s, 1, stdout);
	}
}
