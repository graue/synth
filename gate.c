#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"
#include "rate.inc"

/* gate.c: poor man's gate */

/*
 * You give this program a time in milliseconds, and a sample
 * magnitude value, mag, with 1.0 being full-scale. If every
 * sample has a magnitude less than mag for the time you give,
 * then all samples after that are zeroed out until one with
 * larger or equal magnitude compared to mag is found.
 *
 * or with -drop the samples are just dropped.
 */

static void gate(float len, float threshdB, int drop);

int main(int argc, char *argv[])
{
	float len = 50.0f;
	float threshdB = -80.0f;
	int i;
	int drop = 0;

	get_rate();

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-len") == 0 && i+1 < argc)
			len = atof(argv[++i]);
		else if (strcmp(argv[i], "-threshdB") == 0 && i+1 < argc)
			threshdB = atof(argv[++i]);
		else if (strcmp(argv[i], "-drop") == 0)
			drop = 1;
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -len ms, -threshdB -dB, \n"
				"-drop\n");
			exit(0);
		}
	}

	SET_BINARY_MODE
	gate(len, threshdB, drop);
	return 0;
}

#define M_LN10_OVER_20 0.115129254649702284200899573
#define DBTORAT(x) exp((x) * M_LN10_OVER_20)

static void gate(float len, float threshdB, int drop)
{
	float f[2], avg;
	int smplen = (int)(RATE * len / 1000.0f);
	int numlow = 0;
	int closed = 0;
	float mag = DBTORAT(threshdB);

	while (fread(f, sizeof f[0], 2, stdin) == 2)
	{
		avg = (f[0]+f[1]) / 2;

		if (closed)
		{
			if (fabs(avg) > mag)
				closed = 0;
			else
			{
				avg = f[0] = f[1] = 0.0;
				if (drop)
					continue;
			}
		}
		else if (fabs(avg) < mag && ++numlow == smplen)
		{
			numlow = 0;
			closed = 1;
		}

		if (fwrite(f, sizeof f[0], 2, stdout) < 2)
			return;
	}
}
