#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "binary.h"

/* pan.c: adjust a waveform's horizontal panning */

#ifndef PI
#define PI 3.1415927f
#endif

#define CLAMP(min, n, max) ((n)<(min)?(min):((n)>(max)?(max):(n)))
static void pan(float angle);

int main(int argc, char *argv[])
{
	float angle = 0.0f;
	int i;

	SET_BINARY_MODE

	/* read options */
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-angle") && ++i < argc)
			angle = atof(argv[i]);
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -angle arg (-90 to 90)\n");
			exit(0);
		}
	}

	/* check options */
	angle = CLAMP(-180.0f, angle, 180.0f);

	/* convert options */
	angle = angle * PI / 180.0f;

	pan(angle);
	return 0;
}

static void pan(float angle)
{
	float f;
	float leftamp, rightamp;
	int odd = 0;

	leftamp  = cos(angle) + sin(angle);
	rightamp = cos(angle) - sin(angle);

	for (;;)
	{
		odd = !odd;

		if (fread(&f, sizeof f, 1, stdin) < 1)
			return;

		if (odd) /* left speaker */
			f *= leftamp;
		else     /* right speaker */
			f *= rightamp;

		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			return;
	}
}
