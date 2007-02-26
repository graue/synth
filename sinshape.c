#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"

/* sinshape.c: sinusoid waveshaping */

static void sinshape(void);

int main(int argc, char *argv[])
{
	int i;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-help") == 0)
		{
			fprintf(stderr, "options: none\n");
			exit(0);
		}
	}

	SET_BINARY_MODE
	sinshape();
	return 0;
}

static void sinshape(void)
{
	float f;
	while (fread(&f, sizeof f, 1, stdin) == 1)
	{
		f = sin(f * M_PI); /* map [-1,1] to [-pi,pi] then take sine */
		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			return;
	}
}
