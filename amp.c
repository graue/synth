#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "binary.h"

/* amp.c: adjust a waveform's volume */

#define CLAMP(min, n, max) ((n)<(min)?(min):((n)>(max)?(max):(n)))
static void amp(float volume);

int main(int argc, char *argv[])
{
	float vol = 1.0f;
	int i;

	SET_BINARY_MODE
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-vol") && ++i < argc)
			vol = atof(argv[i]);
	}

	vol = CLAMP(0.0f, vol, 100.0f);

	amp(vol);
	return 0;
}

static void amp(float volume)
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

		f *= volume;

		for (i = 0; i < (int)sizeof (float); i++)
			putchar(fw[i]);
	}
}
