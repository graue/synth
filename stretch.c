#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"

/* stretch: play at a different frequency */

static void stretch(void);

/* The rate is a multiple of normal speed, i.e. 2.0 means 2x as fast. */
static double rate = 2.0f;

int main(int argc, char *argv[])
{
	int i;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-rate") && i+1 < argc)
			rate = strtod(argv[++i], NULL);
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -rate speedratio\n");
			exit(0);
		}
	}

	SET_BINARY_MODE
	stretch();
	return 0;
}

static double framesowed = 0.0f;

/* return next frame or 0 if out of frames */
static float *nextframe(void)
{
	static float f[2];
	if (fread(f, sizeof f[0], 2, stdin) < 2)
		f[0] = f[1] = 0.0f;
	else
		framesowed += 1.0f / rate;
	return f;
}

/* formula copied from Cyanphase MiniTrk (example buzz tracker); CSI? */
static float interpolate(float minusone, float zero, float one, float two,
	double fracpart)
{
	return ((((3.0f * (zero - one) - minusone + two) * 0.5f * fracpart)
		+ 2.0f * one + minusone - (5.0f * zero + two) * 0.5f)
		* fracpart + (one - minusone) * 0.5f) * fracpart + zero;

	/* Other formulas that could be used here:
	 *
	 * Linear:
	 * return fracpart*one + (1.0-fracpart)*zero;
	 * return zero + fracpart*(one-zero);
	 *
	 * Drop-sample:
	 * return one;
	 */
}

static void stretch(void)
{
	float out[2];
	float buf[4][2];
	double pos = 0.0f;

	buf[0][0] = buf[0][1] = 0.0f;
	memcpy(buf[1], nextframe(), sizeof buf[1]);
	memcpy(buf[2], nextframe(), sizeof buf[2]);
	memcpy(buf[3], nextframe(), sizeof buf[3]);

	while (framesowed > 0.0f || !feof(stdin))
	{
		while (pos >= 1.0f)
		{
			memmove(buf[0], buf[1], 3 * sizeof buf[0]);
			memcpy(buf[3], nextframe(), sizeof buf[3]);
			pos -= 1.0f;
		}

		out[0] = interpolate(buf[0][0], buf[1][0], buf[2][0],
			buf[3][0], pos);
		out[1] = interpolate(buf[0][1], buf[1][1], buf[2][1],
			buf[3][1], pos);

		if (fwrite(out, sizeof out[0], 2, stdout) < 1)
			return;
		framesowed -= 1.0f;
		pos += rate;
	}
}
