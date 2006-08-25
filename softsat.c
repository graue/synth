#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"

/* softsat.c: softly saturate a waveform */

static void softsat(float range, float gradation);

int main(int argc, char *argv[])
{
	float range = 1.0f;
	float gradation = 0.5f;
	int i;

	SET_BINARY_MODE

	/* read options */
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-range") && ++i < argc)
			range = atof(argv[i]);
		else if (!strcmp(argv[i], "-hardness") && ++i < argc)
			gradation = atof(argv[i]);
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -range arg, -hardness arg\n");
			exit(0);
		}
	}

	/* check options */
	range     = CLAMP(0.01f, range, 2.0f);
	gradation = CLAMP(0.0f, gradation, 0.99f);

	/* convert options */
	range /= (gradation + 1) / 2; /* 0.75f for the default 0.5f */

	softsat(range, gradation);
	return 0;
}

#define SHAPE							\
	if (f > gradation)					\
	{							\
		f = gradation + (f-gradation)			\
			/ (1+((f-gradation)/(1-gradation))	\
			   * ((f-gradation)/(1-gradation)));	\
	}							\
	if (f > 1)						\
		f = (gradation + 1) / 2;

static void softsat(float range, float gradation)
{
	float f;
	float gradinv;

	gradinv = 1.0f/gradation;
	range *= 32768.0f;

	for (;;)
	{
		if (fread(&f, sizeof f, 1, stdin) < 1)
			return;

		f /= range;

		if (f > 0.0f)
		{
			SHAPE
		}
		else
		{
			f = -f;
			SHAPE
			f = -f;
		}

		f *= range;

		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			return;
	}
}
