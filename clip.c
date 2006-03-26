#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "binary.h"

/* clip.c: clip a waveform */

#define CLAMP(min, n, max) ((n)<(min)?(min):((n)>(max)?(max):(n)))
static void clip(float max, float min);

int main(int argc, char *argv[])
{
	float max = 32768.0f;
	int i;

	SET_BINARY_MODE
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-amp") && ++i < argc)
			max = atof(argv[i]) * 32768.0f;
	}

	clip(max, -max);
	return 0;
}

static void clip(float max, float min)
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

		f = CLAMP(min, f, max);

		for (i = 0; i < (int)sizeof (float); i++)
			putchar(fw[i]);
	}
}
