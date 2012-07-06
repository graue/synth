#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "synth.h"

/* stwidth.c: adjust stereo width */

static void stwidth(float width);

int main(int argc, char *argv[])
{
	float width = 1.0f;
	int i;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-width") && i+1 < argc)
			width = atof(argv[++i]);
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -width width "
				"(1.0 = no change, 0.0 = mono)\n");
			exit(0);
		}
	}

	SET_BINARY_MODE
	stwidth(width);
	return 0;
}

static void stwidth(float width)
{
	float f[2];
	float scale = width * 0.5f;
	float mid, side;

	while (fread(f, sizeof f[0], 2, stdin) == 2)
	{
		mid = 0.5f * (f[0] + f[1]);
		side = scale * (f[1] - f[0]);

		f[0] = mid - side;
		f[1] = mid + side;

		if (fwrite(f, sizeof f[0], 2, stdout) < 2)
			return;
	}
}
