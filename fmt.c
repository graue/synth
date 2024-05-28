#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synth.h"
#define MT_IMPLEMENT
#include "lib/mt.h"

/* fmt: convert floats on stdin to their final format */

/* types of dither: none, rectangular, triangular */
enum { DI_NONE, DI_RECT, DI_TRI };

static void conv(int bits, int sign, int dithertype, int mono, int truncbits,
	int clipwarn);
static int nextsample(double *dst, int monomix);
static double dithernoise(int type);

int main(int argc, char *argv[])
{
	int bits = 16;
	int sign = 1;
	int dithertype = DI_NONE;
	int mono = 0;
	int truncbits = 0;
	int clipwarn = 0;
	int i;

	/* read options */
	for (i = 1; i < argc; i++)
	{
		     if (!strcmp(argv[i], "-u8")) bits = 8, sign = 0;
		else if (!strcmp(argv[i], "-s8")) bits = 8;
		else if (!strcmp(argv[i], "-16")) bits = 16;
		else if (!strcmp(argv[i], "-24")) bits = 24;
		else if (!strcmp(argv[i], "-32")) bits = 32;
		else if (!strcmp(argv[i], "-mono")) mono = 1;
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

	conv(bits, sign, dithertype, mono, truncbits, clipwarn);

	return 0;
}

static void conv(int bits, int sign, int dithertype, int mono, int truncbits,
	int clipwarn)
{
	unsigned int truncmask = 0xffffffffu;
	unsigned int range;
	int max, min;
	double d;
	unsigned int u;
	int *pn = (int *)&u;

	/*
	 * The -truncate N option sets the lowest N samples to 0, so
	 * for example 'fmt -24 -truncate 4' outputs a signal with 20
	 * significant bits.
	 */
	if (truncbits > 0)
	{
		truncmask = (0xffffffffu >> (unsigned int)truncbits)
			<< (unsigned int)truncbits;
		dithertype = DI_NONE; /* don't dither with -truncate */
	}

	range = 1u << (unsigned int)(bits-1);
	max = range - 1;
	min = -max - 1; /* careful: avoid intermediate overflow for 32-bit */

	while (nextsample(&d, mono))
	{
		int clipped = 1;

		d *= range;

		if (dithertype != DI_NONE)
			d += dithernoise(dithertype);

		/* Clip. */
		if (d < (double)min)
			d = min;
		else if (d > (double)max)
			d = max;
		else clipped = 0;

		if (clipwarn && clipped)
		{
			fprintf(stderr, "fmt: clipping occurred\n");
			clipwarn = 0; /* don't warn again */
		}

		*pn = (int)d;
		u &= truncmask;

		if (!sign)
		{
			if (u < (unsigned int)range)
				u += (unsigned int)range;
			else
				u -= (unsigned int)range;
		}

		if (putchar(u & 0xffu) == EOF) break;
		if (bits >= 16 && putchar((u>>8u) & 0xffu) == EOF) break;
		if (bits >= 24 && putchar((u>>16u) & 0xffu) == EOF) break;
		if (bits == 32 && putchar((u>>24u) & 0xffu) == EOF) break;
	}
}

/* returns 1 on success */
static int nextsample(double *dst, int monomix)
{
	float f[2];

	if (monomix)
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

double dithernoise(int type)
{
	if (type == DI_RECT) /* 1-bit rectangular dither */
	{
		/* random value in [-1, 1) */
		return mt_frand() * 2.0 - 1.0;
	}
	else /* DI_TRI: triangular dither */
	{
		return (mt_frand() + mt_frand()) * 2.0 - 2.0;
	}
}
