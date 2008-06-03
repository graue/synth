#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synth.h"

/* ringmod.c: ring mod waveforms (works like mix.c) */

/* TODO: filter and fix aliasing, I guess. */

#define CLAMP(min, n, max) ((n)<(min)?(min):((n)>(max)?(max):(n)))
#define MAXINPUTS 288
static void ring(FILE **inputs, int numfiles);

int main(int argc, char *argv[])
{
	float max = 32768.0f;
	int i;
	FILE *inputs[MAXINPUTS];
	int numfiles = 0;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-amp"))
		{
			if (++i < argc)
				max = atof(argv[i]) * 32768.0f;
		}
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -amp overall_mul\n");
			fprintf(stderr, "Also list 1 to %d input filenames.\n",
				MAXINPUTS);
			exit(0);
		}
		else if (argv[i][0] != '-')
		{
			if (numfiles == MAXINPUTS)
			{
				fprintf(stderr, "ringmod: too many files\n");
				return 1;
			}
			inputs[numfiles] = fopen(argv[i], "rb");
			if (inputs[numfiles] == NULL)
			{
				fprintf(stderr, "ringmod: can't open %s\n",
					argv[i]);
			}
			else
				numfiles++;
		}
	}
	SET_BINARY_MODE

	if (numfiles == 0)
	{
		fprintf(stderr, "ringmod: no files\n");
		return 1;
	}

	ring(inputs, numfiles);

	for (i = 0; i < numfiles; i++)
		fclose(inputs[i]);

	return 0;
}

static void ring(FILE **inputs, int numfiles)
{
	float f;
	int i;
	int filedone[MAXINPUTS];
	int filesleft = numfiles;

	memset(filedone, 0, numfiles * sizeof (int));

	for (;;)
	{
		double prod = 1.0f;

		for (i = 0; i < numfiles; i++)
		{
			if (filedone[i])
				continue;

			if (fread(&f, sizeof f, 1, inputs[i]) < 1)
			{
				filedone[i] = 1;
				filesleft--;
				if (filesleft == 0)
					return;
				continue;
			}

			prod *= (double)f / 32767.0f;
		}

		f = prod * 32767.0f;
		fwrite(&f, sizeof f, 1, stdout);
	}
}
