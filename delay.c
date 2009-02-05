#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synth.h"
#include "rate.inc"

/* delay.c: a delay line */

static void delay(int len_in_smp, float feedback, float dryout, float wetout);

int main(int argc, char *argv[])
{
	float len = 50.0f;
	float feedback = 37.5f;
	float wetout = 100.0f;
	float dryout = 100.0f;
	int len_in_smp;
	int i;

	get_rate();

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-len") && i+1 < argc)
			len = atof(argv[++i]);
		if (!strcmp(argv[i], "-feedback") && i+1 < argc)
			feedback = atof(argv[++i]);
		if (!strcmp(argv[i], "-wetout") && i+1 < argc)
			wetout = atof(argv[++i]);
		if (!strcmp(argv[i], "-dryout") && i+1 < argc)
			dryout = atof(argv[++i]);
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -len ms, -feedback "
				"percent, -wetout percent, -dryout percent\n");
			exit(0);
		}
	}

	len = CLAMP(0.0f, len, 30000.0f);
	len_in_smp = (int)(len * RATE / 1000.0f);
	if (len_in_smp == 0)
		len_in_smp = 1;

	SET_BINARY_MODE
	delay(len_in_smp, feedback, wetout, dryout);
	return 0;
}

static void delay(int len_in_smp, float feedback, float wetout, float dryout)
{
	float *buf;
	float f, f0;
	int i, bufpos = 0;

	len_in_smp *= 2; /* account for stereo */

	/* convert percentages to ratios */
	feedback /= 100.0f;
	wetout /= 100.0f;
	dryout /= 100.0f;

	buf = malloc(sizeof *buf * len_in_smp);
	if (buf == NULL)
	{
		fprintf(stderr, "out of memory\n");
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < len_in_smp; i++)
		buf[i] = 0.0f;

	while (fread(&f0, sizeof f0, 1, stdin) == 1)
	{
		f = dryout*f0 + wetout*buf[bufpos];

		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			return;

		buf[bufpos] = feedback*buf[bufpos] + f0;
		if (++bufpos == len_in_smp)
			bufpos = 0;
	}

	free(buf);
}
