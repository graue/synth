#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"

/* comp.c: compressor (peak only for now, rms stuff not implemented yet) */

static void compress(float threshdB, float ratio, float attack, float release,
	int rms);

int main(int argc, char *argv[])
{
	float threshdB = 0.0f;
	float ratio = 1.0f;
	float attack = 10.0f;
	float release = 100.0f;
	int rms = 0;
	int i;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-threshdB") == 0 && ++i < argc)
			threshdB = atof(argv[i]);
		else if (strcmp(argv[i], "-ratio") == 0 && ++i < argc)
			ratio = atof(argv[i]);
		else if (strcmp(argv[i], "-attack") == 0 && ++i < argc)
			attack = atof(argv[i]);
		else if (strcmp(argv[i], "-release") == 0 && ++i < argc)
			release = atof(argv[i]);
		else if (strcmp(argv[i], "-rms") == 0)
			rms = 1;
		else if (strcmp(argv[i], "-help") == 0)
		{
			fprintf(stderr, "options: -threshdB dB, "
				"-ratio ratio, -attack ms, -release ms, "
				"-rms\n");
			exit(0);
		}
	}

	SET_BINARY_MODE
	compress(threshdB, ratio, attack, release, rms);
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

#define RMS_TIME_MS 5.0f /* RMS averaging time in milliseconds */

static void compress(float threshdB, float ratio, float attack, float release,
	int rms)
{
	float f[2];
	float envdB = DC_OFFSET;
	const float attcoef = ENV_COEF(attack);
	const float relcoef = ENV_COEF(release);

	while (fread(f, sizeof f[0], 2, stdin) == 2)
	{
		float rect[2];
		float link, linkdB;
		float overdB;
		float mul;

		rect[0] = fabs(f[0]);
		rect[1] = fabs(f[1]);
		link = rect[0] > rect[1] ? rect[0] : rect[1];
		link /= 32768.0f;
		link += DC_OFFSET; /* to avoid log(0) */
		linkdB = RATTODB(link);

		overdB = linkdB - threshdB;
		if (overdB < 0.0f) overdB = 0.0f;

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
