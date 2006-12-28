#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synth.h"

/* clip.c: clip a waveform */

static void clip(float max, float min);

int main(int argc, char *argv[])
{
	float max = 1.0f;
	int i;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-amp") && i+1 < argc)
			max = atof(argv[++i]);
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -amp amplitude\n");
			exit(0);
		}
	}

	SET_BINARY_MODE
	clip(max, -max);
	return 0;
}

static void clip(float max, float min)
{
	float f;

	while (fread(&f, sizeof f, 1, stdin) == 1)
	{
		f = CLAMP(min, f, max);

		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			return;
	}
}
