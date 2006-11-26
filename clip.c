#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synth.h"

/* clip.c: clip a waveform */

static void clip(float max, float min);

int main(int argc, char *argv[])
{
	float max = 32768.0f;
	int i;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-amp") && ++i < argc)
			max = atof(argv[i]) * 32768.0f;
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

	for (;;)
	{
		if (fread(&f, sizeof f, 1, stdin) < 1)
			return;

		f = CLAMP(min, f, max);

		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			return;
	}
}
