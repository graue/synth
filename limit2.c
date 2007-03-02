#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"

/* limit2.c: limiter */

static void limit(float threshdB, float attack, float release);

int main(int argc, char *argv[])
{
	float threshdB = 0.0f;
	float attack = 10.0f;
	float release = 100.0f;
	int i;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-threshdB") == 0 && i+1 < argc)
			threshdB = atof(argv[++i]);
		else if (strcmp(argv[i], "-attack") == 0 && i+1 < argc)
			attack = atof(argv[++i]);
		else if (strcmp(argv[i], "-release") == 0 && i+1 < argc)
			release = atof(argv[++i]);
		else if (strcmp(argv[i], "-help") == 0)
		{
			fprintf(stderr, "options: -threshdB dB, "
				"-attack ms, -release ms\n");
			exit(0);
		}
	}

	SET_BINARY_MODE
	limit(threshdB, attack, release);
	return 0;
}

#define DC_OFFSET 1.19209290E-07F /* <float.h>'s FLT_EPSILON */

/*
 * The coefficient to use for an envelope with given time constant.
 * "Faster" than the one comp.c uses -- rises to 99% of in value over duration
 * of time constant.
 */
#define FASTENV_COEF(ms) pow(0.01, (1000.0 / ((ms) * RATE)))

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

#define BUFLEN 2048 /* should be power of 2 */
#define BUFMASK (BUFLEN-1)
#define MINTHRESH 0.0001f

static void limit(float threshdB, float attack, float release)
{
	float in[2], out[2];
	float env;
	const float attcoef = FASTENV_COEF(attack);
	const float relcoef = FASTENV_COEF(release);
	float buf[2][BUFLEN];
	float thresh;
	int peakholdlen;
	int peakheldtime = 0;
	float peak = 0;
	int bufofs = 0;
	int ix;

	/* Empty the buffer. */
	for (ix = 0; ix < BUFLEN; ix++)
		buf[0][ix] = buf[1][ix] = 0.0f;

	peakholdlen = RATE * attack / 1000.0;
	if (peakholdlen < 1)
		peakholdlen = 1;
	else if (peakholdlen >= BUFLEN)
	{
		fprintf(stderr, "limit2: attack is too long\n");
		exit(EXIT_FAILURE);
	}

	thresh = DBTORAT(threshdB);
	if (thresh < MINTHRESH)
	{
		thresh = MINTHRESH;
		threshdB = RATTODB(thresh);
	}
	env = thresh;
	peak = thresh;

	while (fread(in, sizeof in[0], 2, stdin) == 2)
	{
		float rect[2];
		float link;
		float mul;

		rect[0] = fabs(in[0]);
		rect[1] = fabs(in[1]);
		link = rect[0] > rect[1] ? rect[0] : rect[1];

		/* Always use at least the threshold value. */
		if (link < thresh)
			link = thresh;

		/*
		 * Has the peak been held as long as it is allowed to be
		 * or is the new sample greater than the peak?
		 */
		if (peakheldtime >= peakholdlen || link > peak)
		{
			/* Then set this as the new peak. */
			peak = link;
			peakheldtime = 0;
		}

		env = attrelenv_run(attcoef, relcoef, peak, env);
		mul = thresh / env;

		out[0] = mul * buf[0][(bufofs - peakholdlen) & BUFMASK];
		out[1] = mul * buf[1][(bufofs - peakholdlen) & BUFMASK];

		buf[0][bufofs] = in[0];
		buf[1][bufofs] = in[1];
		bufofs++;
		bufofs &= BUFMASK;

		if (fwrite(out, sizeof out[0], 2, stdout) < 2)
			return;
	}
}
