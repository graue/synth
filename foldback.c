#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"

/* foldback.c: foldback distortion on a waveform */

static void foldback(float threshold);

int main(int argc, char *argv[])
{
	float thresh = 32768.0f;
	int i;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-threshold") && i+1 < argc)
			thresh = atof(argv[++i]) * 32768.0f;
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -threshold amplitude\n");
			exit(0);
		}
	}

	SET_BINARY_MODE
	foldback(thresh);
	return 0;
}

static void foldback(float threshold)
{
	float f;

	while (fread(&f, sizeof f, 1, stdin) == 1)
	{
		if (f > threshold || f < -threshold)
		{
			f = fabs(fabs(fmod(f - threshold, threshold*4))
				- threshold*2) - threshold;
		}

		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			return;
	}
}
