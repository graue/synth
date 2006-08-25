#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synth.h"

/* delay.c: a delay line */

static void delay(int len_in_smp, float feedback);

int main(int argc, char *argv[])
{
	float len = 50.0f;
	float feedback = 37.5f;
	int len_in_smp;
	int i;

	SET_BINARY_MODE
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-len") && ++i < argc)
			len = atof(argv[i]);
		if (!strcmp(argv[i], "-feedback") && ++i < argc)
			feedback = atof(argv[i]);
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -len ms, -feedback "
				"percent\n");
			exit(0);
		}
	}

	len = CLAMP(0.0f, len, 30000.0f);
	len_in_smp = (int)(len * RATE / 1000.0f);
	if (len_in_smp == 0)
		len_in_smp = 1;

	delay(len_in_smp, feedback);
	return 0;
}

static void delay(int len_in_smp, float feedback)
{
	float *buf;
	float f, f0;
	int i, bufpos = 0;

	len_in_smp *= 2; /* account for stereo */
	feedback /= 100.0f; /* convert percentage to ratio */

	buf = malloc(sizeof *buf * len_in_smp);
	if (buf == NULL)
	{
		fprintf(stderr, "out of memory\n");
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < len_in_smp; i++)
		buf[i] = 0.0f;

	for (;;)
	{
		if (fread(&f0, sizeof f0, 1, stdin) < 1)
			return;

		f = f0 + buf[bufpos];

		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			return;

		buf[bufpos] = feedback*buf[bufpos] + f0;
		if (++bufpos == len_in_smp)
			bufpos = 0;
	}

	free(buf);
}
