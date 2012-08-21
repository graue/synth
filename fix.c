#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"

/* fix.c: fix NANs and infinities */

static void fix(int verbose);

int main(int argc, char *argv[])
{
	int i, verbose = 0;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-verbose") == 0)
			verbose = 1;
		else if (strcmp(argv[i], "-help") == 0)
		{
			fprintf(stderr, "options: -verbose\n");
			exit(0);
		}
	}

	SET_BINARY_MODE
	fix(verbose);
	return 0;
}

static void fix(int verbose)
{
	float f;
	unsigned long nans = 0, plusinfs = 0, minusinfs = 0, total = 0;

	while (fread(&f, sizeof f, 1, stdin) == 1)
	{
		if (!isfinite(f))
		{
			f = 0.0f;
			if (verbose) /* Keep stats on infinities and NaNs. */
			{
				if (f == INFINITY)
					plusinfs++;
				else if (f == -INFINITY)
					minusinfs++;
				else
					nans++;

				if (!total)
					fprintf(stderr, "Non-finite number "
						"encountered\n");

				total++;
			}
		}
		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			break;
	}

	if (verbose)
		fprintf(stderr, "fix: %lu +Infs, %lu -Infs, %lu NaNs\n",
			plusinfs, minusinfs, nans);
}
