#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "binary.h"

/* unfmt: convert integers on stdin to internal float format */

static void conv_u8(void);
static void conv_s8(void);
static void conv_16(void);
static void conv_24(void);
static void conv_32(void);

int main(int argc, char *argv[])
{
	int fmt = 2;
	int i;

	SET_BINARY_MODE

	/* read options */
	for (i = 1; i < argc; i++)
	{
		     if (!strcmp(argv[i], "-u8")) fmt = 0; /* 8-bit unsigned */
		else if (!strcmp(argv[i], "-s8")) fmt = 1; /* 8-bit signed */
		else if (!strcmp(argv[i], "-16")) fmt = 2; /* 16-bit signed */
		else if (!strcmp(argv[i], "-24")) fmt = 3; /* 24-bit signed */
		else if (!strcmp(argv[i], "-32")) fmt = 4; /* 32-bit signed */
	}

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

static void conv_u8(void)
{
	float f;
	unsigned char s;
	char *fw;
	int c, i;

	fw = (char *)&f;

	for (;;)
	{
		c = getchar();
		if (c == EOF)
			return;
		s = (unsigned char)c;
		f = s;

		/* move range from 0 .. 255 to -128 .. 127 */
		f -= 128.0f;

		/* expand range from -128 .. 127 to -32768 .. 32767 */
		f *= 256.0f;

		for (i = 0; i < (int)sizeof (float); i++)
			putchar(fw[i]);
	}
}

static void conv_s8(void)
{
	float f;
	signed char s;
	char *fw;
	int c, i;

	fw = (char *)&f;

	for (;;)
	{
		c = getchar();
		if (c == EOF)
			return;
		s = (signed char)c;
		f = s;

		/* expand range from -128 .. 127 to -32768 .. 32767 */
		f *= 256.0f;

		for (i = 0; i < (int)sizeof (float); i++)
			putchar(fw[i]);
	}
}

static void conv_16(void)
{
	float f;
	short s;
	char *fw;
	char *sr;
	int c, i;

	fw = (char *)&f;
	sr = (char *)&s;

	for (;;)
	{
		c = getchar();
		if (c == EOF)
			return;
		sr[0] = c;
		c = getchar();
		if (c == EOF)
			return;
		sr[1] = c;

		f = s;

		for (i = 0; i < (int)sizeof (float); i++)
			putchar(fw[i]);
	}
}

static void conv_24(void)
{
	float f;
	double d;
	int s;
	char *fw;
	char *sr;
	int c, i;

	fw = (char *)&f;
	sr = (char *)&s;

	for (;;)
	{
		c = getchar();
		if (c == EOF)
			return;
		sr[0] = c;
		c = getchar();
		if (c == EOF)
			return;
		sr[1] = c;
		c = getchar();
		if (c == EOF)
			return;
		sr[2] = c;
		if (sr[2] & 0x80)
			sr[3] = 0xff;
		else
			sr[3] = 0;

		d = (double)s;

		/* squish range of samples from -8388608 .. 8388607
		   to -32768 .. 32767 */
		d /= 256.0f;

		f = (float)d;

		for (i = 0; i < (int)sizeof (float); i++)
			putchar(fw[i]);
	}
}

static void conv_32(void)
{
	float f;
	double d;
	int s;
	char *fw;
	char *sr;
	int c, i;

	fw = (char *)&f;
	sr = (char *)&s;

	for (;;)
	{
		c = getchar();
		if (c == EOF)
			return;
		sr[0] = c;
		c = getchar();
		if (c == EOF)
			return;
		sr[1] = c;
		c = getchar();
		if (c == EOF)
			return;
		sr[2] = c;
		c = getchar();
		if (c == EOF)
			return;
		sr[3] = c;

		d = (double)s;

		/* squish range of samples from -2147483648 .. 2147483647
		   to -32768 .. 32767 */
		d /= 32768.0f;

		f = (float)d;

		for (i = 0; i < (int)sizeof (float); i++)
			putchar(fw[i]);
	}
}
