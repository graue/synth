#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"

/* gate.c: poor man's gate */

/*
 * You give this program a time in milliseconds, and a sample
 * magnitude value, mag, with 1.0 being full-scale. If every
 * sample has a magnitude less than mag for the time you give,
 * then all samples after that are dropped until one with
 * larger or equal magnitude compared to mag is found.
 */

static void gate(float len, float mag);

int main(int argc, char *argv[])
{
	float len = 50.0f;
	float mag = 0.001f;
	int i;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-len") == 0 && ++i < argc)
			len = atof(argv[i]);
		else if (strcmp(argv[i], "-mag") == 0 && ++i < argc)
			mag = atof(argv[i]);
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -len ms, -mag sampval\n");
			exit(0);
		}
	}

	SET_BINARY_MODE
	gate(len, mag);
	return 0;
}

static void gate(float len, float mag)
{
	float f;
	int smplen = (int)(RATE * len / 1000.0f);
	int numlow = 0;
	int closed = 0;

	while (fread(&f, sizeof f, 1, stdin) == 1)
	{
		if (closed)
		{
			if (fabsf(f) < mag)
				continue;
			closed = 0;
		}
		else if (fabsf(f) < mag && ++numlow == smplen)
		{
			numlow = 0;
			closed = 1;
		}

		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			return;
	}
}
