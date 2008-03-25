#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synth.h"
#define MT_IMPLEMENT
#include "mt.h"

/* fmt: convert floats on stdin to their final format */

/* types of dither: none, rectangular, triangular */
enum { DI_NONE, DI_RECT, DI_TRI };

static int truncbits = 0;
static unsigned int truncmask = 0xffffffff;
static int monodst = 0;
static int dithertype = DI_NONE;
static int clipwarn = 0;
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
		else if (!strcmp(argv[i], "-dither")) dithertype = DI_RECT;
		else if (!strcmp(argv[i], "-tridither")) dithertype = DI_TRI;
		else if (!strcmp(argv[i], "-truncate") && i+1 < argc)
			truncbits = atoi(argv[++i]);
		else if (!strcmp(argv[i], "-clipwarn")) clipwarn = 1;
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -u8, -s8, -16, -24, -32, "
				"-mono, -dither, -tridither, "
				"-truncate numbits,\n"
				"-clipwarn\n");
			exit(0);
		}
	}

	/*
	 * The -truncate N option sets the lowest N samples to 0, so
	 * for example 'fmt -24 -truncate 4' outputs a signal with 20
	 * significant bits.
	 */
	if (truncbits > 0)
	{
		truncmask >>= truncbits;
		truncmask <<= truncbits;
		dithertype = DI_NONE; /* XXX don't dither with -truncate */
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

double dithernoise(void)
{
	if (dithertype == DI_RECT) /* 1-bit rectangular dither */
	{
		/* random value in [-1, 1) */
		return mt_frand() * 2.0 - 1.0;
	}
	else /* DI_TRI: triangular dither */
	{
		return (mt_frand() + mt_frand()) * 2.0 - 2.0;
	}
}

static void clip_warning(void)
{
	fprintf(stderr, "fmt: clipping occurred\n");
	clipwarn = 0; /* don't warn again */
}

static void conv_u8(void)
{
	double f;
	unsigned char s;

	while (nextsample(&f))
	{
		int clipped = 1;

		/* Expand range from [-1, 1] to about [-128, 127]. */
		f *= 128.0;

		/* Move range from -128 .. 127 to 0 .. 255. */
		f += 128.0;

		/* Dither. */
		if (dithertype != DI_NONE)
			f += dithernoise();

		/* Clip. */
		if (f < 0.0)
			f = 0.0;
		else if (f > 255.0)
			f = 255.0;
		else clipped = 0;

		s = (unsigned char)f;
		s &= truncmask;
		putchar((char)s);

		if (clipped && clipwarn)
			clip_warning();
	}
}

static void conv_s8(void)
{
	double f;
	signed char s;

	while (nextsample(&f))
	{
		int clipped = 1;

		/* Expand range from [-1, 1] to about [-128, 127]. */
		f *= 128.0;

		/* Dither. */
		if (dithertype != DI_NONE)
			f += dithernoise();

		/* Clip. */
		if (f < -128.0)
			f = -128.0;
		else if (f > 127.0)
			f = 127.0;
		else clipped = 0;

		s = (signed char)f;
		s &= truncmask;
		putchar((char)s);

		if (clipped && clipwarn)
			clip_warning();
	}
}

static void conv_16(void)
{
	double f;
	short s;

	while (nextsample(&f))
	{
		int clipped = 1;

		/* Expand range from [-1, 1] to about [-32768, 32767]. */
		f *= 32768.0;

		/* Dither. */
		if (dithertype != DI_NONE)
			f += dithernoise();

		/* Clip. */
		if (f < -32768.0)
			f = -32768.0;
		else if (f > 32767.0)
			f = 32767.0;
		else clipped = 0;

		s = (short)f;
		s &= truncmask;
		fwrite(&s, sizeof s, 1, stdout);

		if (clipped && clipwarn)
			clip_warning();
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
		int clipped = 1;

		/* Expand from [-1, 1] to about [-8388608, 8388607]. */
		f *= 8388608.0;

		/* Dither. */
		if (dithertype != DI_NONE)
			f += dithernoise();

		/* Clip. */
		if (f > 8388607.0) f = 8388607.0;
		else if (f < -8388608.0) f = -8388608.f;
		else clipped = 0;

		s = (int)f;
		s &= truncmask;
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

		if (clipped && clipwarn)
			clip_warning();
	}
}

static void conv_32(void)
{
	double f;
	int s;

	while (nextsample(&f))
	{
		int clipped = 1;

		/* Expand from [-1, 1] to about [-2147483648, 2147483647]. */
		f *= 2147483648.0;

		/* Dither. */
		if (dithertype != DI_NONE)
			f += dithernoise();

		/* Clip. */
		if (f > 2147483647.0) f = 2147483647.0;
		else if (f < -2147483648.0) f = -2147483648.f;
		else clipped = 0;

		s = (int)f;
		s &= truncmask;
		fwrite(&s, sizeof s, 1, stdout);

		if (clipped && clipwarn)
			clip_warning();
	}
}
