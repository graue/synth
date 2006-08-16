#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "binary.h"

/* mix.c: mix waveforms */

#define CLAMP(min, n, max) ((n)<(min)?(min):((n)>(max)?(max):(n)))
#define MAXINPUTS 288
static void mix(FILE **inputs, int numfiles);

int main(int argc, char *argv[])
{
	float max = 32768.0f;
	int i;
	FILE *inputs[MAXINPUTS];
	int numfiles = 0;

	SET_BINARY_MODE
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-amp"))
		{
			if (++i < argc)
				max = atof(argv[i]) * 32768.0f;
		}
		else if (argv[i][0] != '-')
		{
			if (numfiles == MAXINPUTS)
			{
				fprintf(stderr, "mix: too many files\n");
				return 1;
			}
			inputs[numfiles] = fopen(argv[i], "rb");
			if (inputs[numfiles] == NULL)
			{
				fprintf(stderr, "mix: can't open %s\n",
					argv[i]);
			}
			else
				numfiles++;
		}
	}

	if (numfiles == 0)
	{
		fprintf(stderr, "mix: no files\n");
		return 1;
	}

	mix(inputs, numfiles);

	for (i = 0; i < numfiles; i++)
		fclose(inputs[i]);

	return 0;
}

static void mix(FILE **inputs, int numfiles)
{
	float f;
	int i;
	int filedone[MAXINPUTS];
	int filesleft = numfiles;

	memset(filedone, 0, numfiles * sizeof (int));

	for (;;)
	{
		float sum = 0.0f;

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

			sum += f;
		}

		f = sum;
		fwrite(&f, sizeof f, 1, stdout);
	}
}
