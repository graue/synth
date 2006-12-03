#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synth.h"

/* fmt: convert floats on stdin to their final format */

static int monodst = 0;
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
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -u8, -s8, -16, -24, -32, "
				"-mono\n");
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
static int nextsample(float *dst)
{
	if (monodst)
	{
		float f[2];
		if (fread(f, sizeof f[0], 2, stdin) < 2)
			return 0;
		*dst = (f[0] + f[1]) / 2.0f;
		return 1;
	}
	return fread(dst, sizeof *dst, 1, stdin) == 1;
}

static void conv_u8(void)
{
	float f;
	unsigned char s;

	while (nextsample(&f))
	{
		/* squish range from -32768 .. 32767 to -128 .. 127 */
		f /= 256.0f;

		/* move range from -128 .. 127 to 0 .. 255 */
		f += 128.0f;

		/* clip */
		if (f < 0.0f)
			f = 0.0f;
		else if (f > 255.0f)
			f = 255.0f;

		s = (unsigned char)f;
		putchar((char)s);
	}
}

static void conv_s8(void)
{
	float f;
	signed char s;

	while (nextsample(&f))
	{
		/* squish range from -32768 .. 32767 to -128 .. 127 */
		f /= 256.0f;

		/* clip */
		if (f < -128.0f)
			f = -128.0f;
		else if (f > 127.0f)
			f = 127.0f;

		s = (signed char)f;
		putchar((char)s);
	}
}

static void conv_16(void)
{
	float f;
	short s;

	while (nextsample(&f))
	{
		/* clip */
		if (f < -32768.0f)
			f = -32768.0f;
		else if (f > 32767.0f)
			f = 32767.0f;

		s = (short)f;
		fwrite(&s, sizeof s, 1, stdout);
	}
}

static void conv_24(void)
{
	float f;
	double d;
	int s;
	unsigned char *sw;

	sw = (unsigned char *)&s;

	while (nextsample(&f))
	{
		d = f;

		/*
		 * Expand range of samples from -32768 .. 32767
		 * to -8388608 .. 8388607.
		 */
		d *= 256.0f;

		/* Clip. */
		if (d > 8388607.0f) d = 8388607.0f;
		else if (d < -8388608.0f) d = -8388608.f;

		s = (int)d;
#if WORDS_BIGENDIAN
		if (sw[0] == 0xff) putc(sw[1] | 0x80, stdout);
		else putc(sw[1], stdout);
		putc(sw[2], stdout);
		putc(sw[3], stdout);
#else
		if (sw[3] == 0xff) putc(sw[2] | 0x80, stdout);
		else putc(sw[2], stdout);
		putc(sw[1], stdout);
		putc(sw[0], stdout);
#endif
	}
}

static void conv_32(void)
{
	float f;
	double d;
	int s;

	while (nextsample(&f))
	{
		/* clip */
		if (f < -32768.0f)
			f = -32768.0f;
		else if (f > 32767.0f)
			f = 32767.0f;

		d = f;

		/*
		 * Expand range of samples from -32768 .. 32767
		 * to -2147483648 .. 2147483647.
		 */
		d *= 32768.0f;

		s = (int)d;
		fwrite(&s, sizeof s, 1, stdout);
	}
}
