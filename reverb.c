#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"
#include "rate.inc"

/* reverb.c: cyan's psg reverb */

/* default values are meant to reflect those from the old optionless reverb */

static void reverb(float wetoutdB, float decayamt, float damping,
	float predelayms, float roomsizems, float density);

int main(int argc, char *argv[])
{
	float wetoutdB = 0.0;
	float decayamt = 0.8;
	float damping = 0.9;
	float roomsizems = 226.75736961451;
	float predelayms = roomsizems / 10;
	float density = 0.8;
	int i;

	get_rate();

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-wetoutdB") && i+1 < argc)
			wetoutdB = atof(argv[++i]);
		else if (!strcmp(argv[i], "-decay") && i+1 < argc)
			decayamt = atof(argv[++i]);
		else if (!strcmp(argv[i], "-damping") && i+1 < argc)
			damping = atof(argv[++i]);
		else if (!strcmp(argv[i], "-roomsize") && i+1 < argc)
			roomsizems = atof(argv[++i]);
		else if (!strcmp(argv[i], "-predelay") && i+1 < argc)
			predelayms = atof(argv[++i]);
		else if (!strcmp(argv[i], "-density") && i+1 < argc)
			density = atof(argv[++i]);
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr,
				"options (useful range) [default]:\n"
				" -wetoutdB [0.0 dB]\n"
				" -decay (0.0 - 1.0) [0.8]\n"
				" -damping (0.0 - 0.95) [0.9]\n"
				" -roomsize [226.76 ms]\n"
				" -predelay [22.676 ms]\n"
				" -density (0.0 - 1.0) [0.8]\n"
				);
			exit(0);
		}
		else
		{
			fprintf(stderr,
				"reverb: disregarding unknown option %s\n",
				argv[i]);
		}
	}

	SET_BINARY_MODE
	reverb(wetoutdB, decayamt, damping, predelayms, roomsizems, density);
	return 0;
}

#define M_20_OVER_LN10 8.68588963806503655302257838
#define M_LN10_OVER_20 0.115129254649702284200899573
#define RATTODB(x) (log(x) * M_20_OVER_LN10)
#define DBTORAT(x) exp((x) * M_LN10_OVER_20)

/*
 * The most reverb stages that will be used. The actual number of
 * stages depends on how much can fit in the memory used.
 */
#define RV_MAX_STAGES 1024

/*
 * How much granularity shall there be in randomly deciding the
 * gain of each reverb stage?
 */
#define RV_GAIN_RANGE 30

/*
 * Reverb memory: increase for a denser sound (more stages).
 * Using a small amount, like 128, will get more of a multi-tap
 * delay type effect, with only about 4 or 5 stages.
 *
 * There are diminishing returns after 2048, the default.
 */
static size_t rv_mem = 2048;

/*
 * Room size in sample frames. This affects how much memory
 * each reverb stage uses.
 */
static int rv_roomsize = 10000;

/*
 * Reverb pre-delay in sample frames. This also affects how
 * much memory each reverb stage uses. Too low of a value
 * might sound bad.
 */
static int rv_predelay = 1000;

/* Decay amount -- set too high if you want terrible feedback. */
static float rv_decay = 0.8f;

/* The extent to which high frequencies are dampened. */
static float rv_damp = 0.9f;

/* Reverb output level. */
static float rv_return = RV_GAIN_RANGE * 0.5f;

typedef struct revstate {
	float *dl; /* The start of the delay lines. */
	int dlstart[RV_MAX_STAGES]; /* Offset to start of each delay line. */
	int dllen[RV_MAX_STAGES]; /* Length of each delay line. */
	int dlpos[RV_MAX_STAGES]; /* Position in delay line, from start. */
	float dlamp[RV_MAX_STAGES]; /* Amplitude of each stage. */
	float flt[RV_MAX_STAGES]; /* Filtering state of each stage. */
} revstate_t;

static revstate_t *do_reverb(revstate_t *rev, float *samps, int nsamps);

static void reverb(float wetoutdB, float decayamt, float damping,
	float predelayms, float roomsizems, float density)
{
	float f[2];
	revstate_t *rev[2] = {NULL, NULL};

	rv_return *= DBTORAT(wetoutdB);
	rv_decay = decayamt;

	/*
	 * In lieu of a way to link it to any measurement that really makes
	 * sense, make the damping value relative to 44.1 KHz since that is
	 * what the effect originally worked at.
	 *
	 * XXX: I don't really know for sure what I'm doing here.
	 */
	rv_damp = CLAMP(0.0, damping * 44100 / RATE, 0.95);

	rv_predelay = (int)(predelayms / 1000.0 * RATE);
	rv_roomsize = (int)(roomsizems / 1000.0 * RATE);

	density = CLAMP(0.0, density, 1.0);
	rv_mem = (int)pow(2, 7 + 5*density);

	while (fread(f, sizeof f[0], 2, stdin) == 2)
	{
		rev[0] = do_reverb(rev[0], f, 1);
		rev[1] = do_reverb(rev[1], f+1, 1);

		if (fwrite(f, sizeof f[0], 2, stdout) < 2)
			return;
	}
}

static void init_revstate(revstate_t *rev)
{
	int ix;
	int memoffset;

	rev->dl = (float *)(((char *)rev) + sizeof *rev);
	memset(rev->dl, 0, rv_mem*RV_MAX_STAGES);

	for (ix = 0, memoffset = 0; ix < RV_MAX_STAGES; ix++)
	{
		rev->flt[ix] = 0;
		rev->dlstart[ix] = memoffset;
		rev->dllen[ix] = (rand()%rv_roomsize)+rv_predelay;
		memoffset += rev->dllen[ix];
		rev->dlpos[ix] = 0;
		rev->dlamp[ix] = (1 + rand() % (RV_GAIN_RANGE-1))
			/ (float)RV_GAIN_RANGE;
		if ((memoffset*sizeof (float)) >= (rv_mem*RV_MAX_STAGES))
		{
			/* Out of memory for reverb stages. */
			rev->dllen[ix] = 0;
			break;
		}
	}
}

void run_reverb(revstate_t *rev, float *samps, int nsamps)
{
	int ix;
	int stage;
	float sum;

	for (ix = 0; ix < nsamps; ix++)
	{
		sum = 0.0f; /* to hold the sum of all stages */
		for (stage = 0; stage < RV_MAX_STAGES; stage++)
		{
			const int offset =
				rev->dlstart[stage] + rev->dlpos[stage];

			if (rev->dllen[stage] == 0) /* no more stages */
				break;

			sum += rev->dlamp[stage] * rev->dl[offset];

			/* Update filter state. */
			rev->flt[stage] =
				rev->flt[stage]*rv_damp
					+ (rev->dl[offset]*(1-rv_damp));

			rev->dl[offset] = samps[ix] * (1-rv_decay)
				+ rev->flt[stage] * rv_decay;
			if (++rev->dlpos[stage] == rev->dllen[stage])
				rev->dlpos[stage] = 0;
		}
		sum /= (float)stage; /* average all stages */
		samps[ix] += sum * rv_return;
	}
}

static revstate_t *do_reverb(revstate_t *rev, float *samps, int nsamps)
{
	if (rev == NULL)
	{
		rev = malloc(sizeof *rev + RV_MAX_STAGES*rv_mem);
		if (rev == NULL)
			fprintf(stderr, "Out of memory, disabling reverb\n");
		else
			init_revstate(rev);
	}
	else run_reverb(rev, samps, nsamps);

	return rev;
}
