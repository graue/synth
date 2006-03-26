#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "binary.h"

/* foldback.c: foldback distortion on a waveform */

#define CLAMP(min, n, max) ((n)<(min)?(min):((n)>(max)?(max):(n)))
static void foldback(float threshold);

int main(int argc, char *argv[])
{
	float thresh = 32768.0f;
	int i;

	SET_BINARY_MODE
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-threshold") && ++i < argc)
			thresh = atof(argv[i]) * 32768.0f;
	}

	foldback(thresh);
	return 0;
}

static void foldback(float threshold)
{
	float f;
	char *fw;
	int c, i;

	fw = (char *)&f;

	for (;;)
	{
		for (i = 0; i < (int)sizeof (float); i++)
		{
			c = getchar();
			if (c == EOF)
				return;
			fw[i] = c;
		}

		if (f > threshold || f < -threshold)
		{
			f = fabs(fabs(fmod(f - threshold, threshold*4))
				- threshold*2) - threshold;
		}

		for (i = 0; i < (int)sizeof (float); i++)
			putchar(fw[i]);
	}
}
