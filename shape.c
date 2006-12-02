#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synth.h"

/* shape.c: waveshape a waveform */

static void shape(float range, float gradation);

int main(int argc, char *argv[])
{
	float range = 1.0f;
	float gradation = 3.0f;
	int i;

	/* read options */
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-range") && ++i < argc)
			range = atof(argv[i]);
		else if (!strcmp(argv[i], "-gradation") && ++i < argc)
			gradation = atof(argv[i]);
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -range arg, -gradation "
				"arg\n");
			exit(0);
		}
	}

	/* check options */
	range     = CLAMP(-2.0f, range, 2.0f);
	gradation = CLAMP(0.1f, gradation, 15.0f);

	SET_BINARY_MODE
	shape(range, gradation);
	return 0;
}

static float shapeval(float val, float r, float g)
{
	float x;
	x = val / 32768.0f;

	if (x < -r/(g+1))
		x = x/g - r/(g+1) * (g-1/g);
	else if (x > r/(g+1))
		x = x/g + r/(g+1) * (g-1/g);
	else
		x *= g;

	return x * 32768.0f;
}

static void shape(float range, float gradation)
{
	float f;

	while (fread(&f, sizeof f, 1, stdin) == 1)
	{
		f = shapeval(f, range, gradation);

		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			return;
	}
}
