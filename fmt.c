#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "binary.h"

/* fmt: convert floats on stdin to their final format */

static void conv_u8(void);
static void conv_s8(void);
static void conv_16(void);
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
		else if (!strcmp(argv[i], "-32")) fmt = 4; /* 32-bit signed */
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -u8, -s8, -16, -32\n");
			exit(0);
		}
	}

	if (fmt == 0)
		conv_u8();
	else if (fmt == 1)
		conv_s8();
	else if (fmt == 2)
		conv_16();
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
		for (i = 0; i < (int)sizeof (float); i++)
		{
			c = getchar();
			if (c == EOF)
				return;
			fw[i] = (char)c;
		}

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
	char *fw;
	int c, i;

	fw = (char *)&f;

	for (;;)
	{
		for (i = 0; i < (int)sizeof (float); i++)
		{
			c = getchar();
			if (c == EOF)
				return;
			fw[i] = (char)c;
		}

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
	char *fw;
	char *sr;
	int c, i;

	fw = (char *)&f;
	sr = (char *)&s;

	for (;;)
	{
		for (i = 0; i < (int)sizeof (float); i++)
		{
			c = getchar();
			if (c == EOF)
				return;
			fw[i] = (char)c;
		}

		/* clip */
		if (f < -32768.0f)
			f = -32768.0f;
		else if (f > 32767.0f)
			f = 32767.0f;

		s = (short)f;
		putchar(sr[0]);
		putchar(sr[1]);
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
		for (i = 0; i < (int)sizeof (float); i++)
		{
			c = getchar();
			if (c == EOF)
				return;
			fw[i] = (char)c;
		}

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
		putchar(sr[0]);
		putchar(sr[1]);
		putchar(sr[2]);
		putchar(sr[3]);
	}
}
