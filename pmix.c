#define _BSD_SOURCE /* get popen, pclose on Linux */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synth.h"

/* pmix.c: mix waveforms but taking commands (piping to stdout) not filenames */

#define MAXINPUTS 288
static void mix(FILE **inputs, int numfiles);

int main(int argc, char *argv[])
{
	int i;
	FILE *inputs[MAXINPUTS];
	int numfiles = 0;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: none\n");
			fprintf(stderr, "Also list 1 to %d input filenames.\n",
				MAXINPUTS);
			exit(0);
		}
		else if (argv[i][0] != '-')
		{
			if (numfiles == MAXINPUTS)
			{
				fprintf(stderr, "pmix: too many files\n");
				return 1;
			}
			inputs[numfiles] = popen(argv[i], "r");
			if (inputs[numfiles] == NULL)
			{
				fprintf(stderr, "pmix: can't run %s\n",
					argv[i]);
			}
			else
				numfiles++;
		}
	}

	if (numfiles == 0)
	{
		fprintf(stderr, "pmix: no files\n");
		return 1;
	}

	SET_BINARY_MODE
	mix(inputs, numfiles);

	for (i = 0; i < numfiles; i++)
		pclose(inputs[i]);

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
