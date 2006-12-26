#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"

/* delay2.c: an overcomplicated delay effect */

static void delay(float len, float feedback, float wetout, float lfolen,
	float lfoamp, float lfoinitphase, int lfomod);

int main(int argc, char *argv[])
{
	float len = 50.0f; /* 50 ms */
	float feedback = 37.5f; /* 37.5% */
	float wetout = 100.0f; /* 100% */
	float lfolen = 100.0f; /* 100 ms */
	float lfoamp = 0.0f; /* 0% */
	float lfoinitphase = 0.0f; /* 0.0 out of 1.0 */
	int lfomod = 0; /* not modulated */
	int i;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-len") && i+1 < argc)
			len = atof(argv[++i]);
		if (!strcmp(argv[i], "-feedback") && i+1 < argc)
			feedback = atof(argv[++i]);
		if (!strcmp(argv[i], "-wetout") && i+1 < argc)
			wetout = atof(argv[++i]);
		if (!strcmp(argv[i], "-lfolen") && i+1 < argc)
			lfolen = atof(argv[++i]);
		if (!strcmp(argv[i], "-lfoamp") && i+1 < argc)
			lfoamp = atof(argv[++i]);
		if (!strcmp(argv[i], "-lfophase") && i+1 < argc)
			lfoinitphase = atof(argv[++i]);
		if (!strcmp(argv[i], "-lfomod"))
			lfomod = 1;
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -len ms, -feedback "
				"percent, -wetout percent,\n");
			fprintf(stderr, "-lfolen ms, -lfoamp percent, "
				"-lfophase 0to1, -lfomod\n");
			exit(0);
		}
	}

	SET_BINARY_MODE
	delay(len, feedback, wetout, lfolen, lfoamp, lfoinitphase, lfomod);
	return 0;
}

static void delay(float len, float feedback, float wetout,
	float lfolen, float lfoamp, float lfoinitphase, int lfomod)
{
	float *buf[2];
	float f, f0[2] = {0.0f, 0.0f};
	int i, bufpos = 0;
	int maxsamples; /* max samples of delay, used for buffer length */
	int side = 0;
	float lfophase;
	float lfoinc;
	float efflen; /* effective length taking lfo into account */

	/*
	 * Convert parameters.
	 */

	lfoamp /= 100.0f; /* convert percentage to ratio */
	lfoamp = CLAMP(0.0f, lfoamp, 1.0f);

	len = CLAMP(0.0f, len, 30000000.0f);
	len *= RATE / 1000.0f; /* convert ms to samples */

	maxsamples = (int)(len * (1.0f + lfoamp)) + 1;

	feedback /= 100.0f; /* convert percentage to ratio */

	lfophase = lfoinitphase;
	lfoinc = 2 * M_PI / (lfolen * RATE / 1000.0f);

	if ((buf[0] = malloc(sizeof *buf[0] * maxsamples)) == NULL
		|| (buf[1] = malloc(sizeof *buf[1] * maxsamples)) == NULL)
	{
		fprintf(stderr, "delay2: out of memory\n");
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < maxsamples; i++)
		buf[0][i] = buf[1][i] = 0.0f;

	efflen = len * (1.0f + lfoamp*sin(lfophase));
	while (fread(&f0[side], sizeof f0[side], 1, stdin) == 1)
	{
		f = f0[side] + wetout*buf[side][bufpos];

		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			return;

		buf[side][(bufpos+maxsamples+(size_t)efflen) % maxsamples]
			= feedback*buf[side][bufpos] + f0[side];
		if (side == 1)
		{
			if (++bufpos == maxsamples)
				bufpos = 0;
			lfophase += lfomod ? (f0[0]+f0[1]) * 0.5f
				: lfoinc;
			efflen = len * (1.0f + lfoamp*sin(lfophase));
		}

		side = !side;
	}

	free(buf[0]);
	free(buf[1]);
}
