#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "synth.h"
#define MT_IMPLEMENT
#include "mt.h"
#include "rate.inc"

/* sampbins.c: randomized sample and hold effect */

static void sampbins(int numbins, int grainsamps);

int main(int argc, char *argv[])
{
	float grainlen = 200.0;
	int numbins = 10;
	int grainsamps;
	int i;

	get_rate();

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-numbins") && i+1 < argc)
			numbins = atoi(argv[++i]);
		else if (!strcmp(argv[i], "-grainlen") && i+1 < argc)
			grainlen = atof(argv[++i]);
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -numbins n, -grainlen ms\n");
			exit(0);
		}
	}

	grainsamps = (int)(grainlen * RATE / 1000.0f);

	if (grainsamps < 30)
	{
		fprintf(stderr, "sampbins: grains too short\n");
		exit(EXIT_FAILURE);
	}
	if (numbins < 1)
	{
		fprintf(stderr, "sampbins: must have at least 1 bin\n");
		exit(EXIT_FAILURE);
	}

	SET_BINARY_MODE
	sampbins(numbins, grainsamps);
	return 0;
}

static void nomem(void)
{
	fprintf(stderr, "sampbins: out of memory\n");
	exit(EXIT_FAILURE);
}

#define FADESAMPS 50

#define M_LN10_OVER_20 0.115129254649702284200899573
#define DBTORAT(x) exp((x) * M_LN10_OVER_20)

static void sampbins(int numbins, int grainsamps)
{
	float **bins;
	int *virgin; /* boolean: this bin written to? */
	float in[2];
	float fadeamp;
	float fademul;
	int ix;
	int cur = 0; /* current bin (playback) */
	int rec = 0; /* current bin (recording) */
	int pos = 0; /* bin pos, in stereo samples */

	bins = malloc(sizeof *bins * numbins);
	if (bins == NULL) nomem();
	virgin = malloc(sizeof *virgin * numbins);
	if (virgin == NULL) nomem();

	for (ix = 0; ix < numbins; ix++)
	{
		bins[ix] = malloc(sizeof **bins * grainsamps * 2);
		if (bins[ix] == NULL) nomem();
		memset(bins[ix], 0, sizeof **bins * grainsamps * 2);
		virgin[ix] = 1;
	}

	mt_init((unsigned int)time(NULL));
	rec = mt_urand() % numbins;

	while (fread(in, sizeof in[0], 2, stdin) == 2)
	{
		/*
		 * first write output sample. this way if
		 * playing from/recording to the same bin,
		 * it works.
		 */
		if (fwrite(&bins[cur][pos*2], sizeof **bins, 2, stdout) < 2)
			return;

		/* record input sample. */
		bins[rec][pos*2] = in[0];
		bins[rec][pos*2+1] = in[1];

		/*
		 * advance position, going to new bins if
		 * necessary.
		 */
		pos++;
		if (pos == grainsamps)
		{
			/*
			 * if grains are long enough, fade
			 * the newly recorded one to avoid
			 * clicking.
			 */
			if (grainsamps > FADESAMPS*2)
			{
				for (ix = 0; ix < FADESAMPS; ix++)
				{
					fadeamp = 96 * (ix / (float)FADESAMPS)
						- 96;
					fademul = DBTORAT(fadeamp);
					bins[rec][ix*2] *= fademul;
					bins[rec][ix*2+1] *= fademul;
					bins[rec][grainsamps-ix*2-2] *= fademul;
					bins[rec][grainsamps-ix*2-1] *= fademul;
				}
			}

			virgin[rec] = 0;
			rec = mt_urand() % numbins;

			/*
			 * find the next bin that's written,
			 * to play from.
			 */
			do
			{
				cur++;
				if (cur == numbins)
					cur = 0;
			} while (virgin[cur]);

			pos = 0;
		}
	}
}
