#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"
#include "rate.inc"

/* comp.c: compressor */

static void compress(float threshdB, float ratio, float attack, float release,
	int rms, float rmswindow, int upward);
static float str_to_ratio(const char *s);

int main(int argc, char *argv[])
{
	float threshdB = 0.0f;
	float ratio = 1.0f;
	float attack = 10.0f;
	float release = 100.0f;
	float rmswindow = 5.0f; /* in milliseconds */
	int upward = 0;
	int rms = 0;
	int i;

	get_rate();

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-threshdB") == 0 && i+1 < argc)
			threshdB = atof(argv[++i]);
		else if (strcmp(argv[i], "-ratio") == 0 && i+1 < argc)
			ratio = str_to_ratio(argv[++i]);
		else if (strcmp(argv[i], "-attack") == 0 && i+1 < argc)
			attack = atof(argv[++i]);
		else if (strcmp(argv[i], "-release") == 0 && i+1 < argc)
			release = atof(argv[++i]);
		else if (strcmp(argv[i], "-rms") == 0)
			rms = 1;
		else if (strcmp(argv[i], "-rmswindow") == 0 && i+1 < argc)
			rmswindow = atof(argv[++i]);
		else if (strcmp(argv[i], "-upward") == 0)
			upward = 1;
		else if (strcmp(argv[i], "-help") == 0)
		{
			fprintf(stderr, "options: -threshdB dB, "
				"-ratio ratio, -attack ms, -release ms, "
				"-rms, -rmswindow ms, -upward\n");
			exit(0);
		}
	}

	if (!isfinite(ratio) || ratio <= 0.0f)
	{
		fprintf(stderr, "comp: ratio must be positive\n");
		exit(EXIT_FAILURE);
	}

	SET_BINARY_MODE
	compress(threshdB, ratio, attack, release, rms, rmswindow, upward);
	return 0;
}

#define DC_OFFSET 1.19209290E-07F /* <float.h>'s FLT_EPSILON */

/* The coefficient to use for an envelope with given time constant. */
#define ENV_COEF(ms) exp(-1000.0f / ((ms) * RATE))

static float env_run(float coef, float in, float state)
{
	return in - coef * (in-state);
}

static float attrelenv_run(float attcoef, float relcoef, float in, float state)
{
	return env_run(in > state ? attcoef : relcoef, in, state);
}

#define M_20_OVER_LN10 8.68588963806503655302257838
#define M_LN10_OVER_20 0.115129254649702284200899573
#define RATTODB(x) (log(x) * M_20_OVER_LN10)
#define DBTORAT(x) exp((x) * M_LN10_OVER_20)

static void compress(float threshdB, float ratio, float attack, float release,
	int rms, float rmswindow, int upward)
{
	float f[2];
	float envdB = DC_OFFSET;
	float rmsstate = DC_OFFSET;
	const float attcoef = ENV_COEF(attack);
	const float relcoef = ENV_COEF(release);
	const float rmscoef = ENV_COEF(rmswindow);

	while (fread(f, sizeof f[0], 2, stdin) == 2)
	{
		float rect[2];
		float link, linkdB;
		float overdB;
		float mul;

		if (rms)
		{
			float avg;

			/* square input */
			rect[0] = f[0] * f[0];
			rect[1] = f[1] * f[1];
			avg = (rect[0] + rect[1]) / 2.0f;
			avg += DC_OFFSET; /* prevent denormal */
			rmsstate = env_run(rmscoef, avg, rmsstate);
			link = sqrt(rmsstate); /* approximate RMS */

			/*
			 * This only approximates RMS, using a 1-pole IIR
			 * instead of an FIR moving average. Close enough.
			 */
		}
		else
		{
			rect[0] = fabs(f[0]);
			rect[1] = fabs(f[1]);
			link = rect[0] > rect[1] ? rect[0] : rect[1];
		}

		link += DC_OFFSET; /* to avoid log(0) */
		linkdB = RATTODB(link);
		overdB = linkdB - threshdB;
		if (upward) {
			if (overdB > 0.0f) overdB = 0.0f;
		} else {
			if (overdB < 0.0f) overdB = 0.0f;
		}

		overdB += DC_OFFSET; /* avoid denormal */
		envdB = attrelenv_run(attcoef, relcoef, overdB, envdB);
		overdB = envdB - DC_OFFSET; /* remove DC offset */

		mul = DBTORAT(overdB * (ratio - 1.0f));
		f[0] *= mul;
		f[1] *= mul;

		if (fwrite(f, sizeof f[0], 2, stdout) < 2)
			return;
	}
}

/*

process a string like x:y, where x and y are decimal numbers,
   returning the float y/x
alternatively if the string has no :, just use atof

*/
static float str_to_ratio(const char *s)
{
	const char *colon;
	float f1, f2;

	colon = strchr(s, ':');
	if (colon == NULL)
		return atof(s);

	f1 = atof(s);
	f2 = atof(colon+1);
	return f2/f1;
}
