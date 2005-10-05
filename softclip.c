#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "binary.h"

/* softclip.c: soft clip a waveform */

#define CLAMP(min, n, max) ((n)<(min)?(min):((n)>(max)?(max):(n)))
static void softclip(float range, float gradation);

int main(int argc, char *argv[])
{
	float range = 1.0f;
	float gradation = 1.0f;
	int i;

	SET_BINARY_MODE

	/* read options */
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-range") && ++i < argc)
			range = atof(argv[i]);
		else if (!strcmp(argv[i], "-gradation") && ++i < argc)
			gradation = atof(argv[i]);
	}

	/* check options */
	range     = CLAMP(-2.0f, range, 2.0f);
	gradation = CLAMP(0.1f, gradation, 15.0f);

	softclip(range, gradation);
	return 0;
}

static void softclip(float range, float gradation)
{
	float f;
	float gradinv;
	char *fw;
	int c, i;

	fw = (char*) &f;
	gradinv = 1.0f/gradation;
	range *= 32768.0f;

	while (1)
	{
		for (i = 0; i < (int) sizeof (float); i++)
		{
			c = getchar();
			if (c == EOF)
				return;
			fw[i] = c;
		}

		f /= range;

		f = gradinv * atan(f * gradation);

		f *= range;

		for (i = 0; i < (int) sizeof (float); i++)
			putchar(fw[i]);
	}
}
