#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"
#include "rate.inc"

/* filter.c: highpass, lowpass, or other IIR filter */

/*
 * See Robert Bristow-Johnson's "cookbook filters" article.
 * This code implements the "cookbook" filters discussed
 * there.
 */

#define CLAMP(min, n, max) ((n)<(min)?(min):((n)>(max)?(max):(n)))

typedef struct
{
	float b0, b1, b2;
	float a0, a1, a2;
} biquadcoeffs_t;

enum
{
	FILT_LP, FILT_HP, FILT_BP_SKIRT_GAIN, FILT_BP_CONSTANT_PEAK,
	FILT_NOTCH, FILT_AP, FILT_PEAKINGEQ, FILT_LOWSHELF, FILT_HIGHSHELF,

	USE_Q, USE_BW, USE_SLOPE
};

static void computecoeffs(biquadcoeffs_t *co, int type, int usewhich,
	float fs, float f0, float dBgain, float q, float bw, float slope);
static void filter(const biquadcoeffs_t *coeffs);

int main(int argc, char *argv[])
{
	float f0; /* set below */
	float q = 1.0f;
	float bw = 1.0f;
	float slope = 1.0f;
	float dBgain = 1.0f;
	int type = FILT_HP;
	int usewhich = USE_Q;
	int i;
	biquadcoeffs_t co;

	get_rate();
	f0 = RATE / 4.0f;

	for (i = 1; i < argc; i++)
	{
		if ((strcmp(argv[i], "-cutoff") == 0
			|| strcmp(argv[i], "-center") == 0
			|| strcmp(argv[i], "-corner") == 0
			|| strcmp(argv[i], "-midpoint") == 0)
			&& i+1 < argc)
		{
			f0 = atof(argv[++i]);
		}
		else if ((strcmp(argv[i], "-dbgain") == 0
			|| strcmp(argv[i], "-dBgain") == 0)
			&& i+1 < argc)
		{
			dBgain = atof(argv[++i]);
			dBgain = CLAMP(-100.0f, q, 100.0f);
		}
		else if (strcmp(argv[i], "-q") == 0 && i+1 < argc)
		{
			q = atof(argv[++i]);
			q = CLAMP(0.01f, q, 100.0f);
			usewhich = USE_Q;
		}
		else if (strcmp(argv[i], "-bw") == 0 && i+1 < argc)
		{
			bw = atof(argv[++i]);
			bw = CLAMP(0.01f, bw, 100.0f);
			usewhich = USE_BW;
		}
		else if (strcmp(argv[i], "-slope") == 0 && i+1 < argc)
		{
			slope = atof(argv[++i]);
			slope = CLAMP(0.01f, slope, 100.0f);
			usewhich = USE_SLOPE;
		}
		else if (strcmp(argv[i], "-type") == 0 && i+1 < argc)
		{
			i++;
			if (strcmp(argv[i], "hp") == 0
				|| strcmp(argv[i], "highpass") == 0
				|| strcmp(argv[i], "hpf") == 0)
			{
				type = FILT_HP;
			}
			else if (strcmp(argv[i], "lp") == 0
				|| strcmp(argv[i], "lowpass") == 0
				|| strcmp(argv[i], "lpf") == 0)
			{
				type = FILT_LP;
			}
			else if (strcmp(argv[i], "bp_skirt_gain") == 0)
				type = FILT_BP_SKIRT_GAIN;
			else if (strcmp(argv[i], "bp") == 0
				|| strcmp(argv[i], "bandpass") == 0
				|| strcmp(argv[i], "bpf") == 0)
			{
				type = FILT_BP_CONSTANT_PEAK;
			}
			else if (strcmp(argv[i], "notch") == 0)
				type = FILT_NOTCH;
			else if (strcmp(argv[i], "ap") == 0
				|| strcmp(argv[i], "allpass") == 0
				|| strcmp(argv[i], "apf") == 0)
			{
				type = FILT_AP;
			}
			else if (strcmp(argv[i], "peakingeq") == 0)
				type = FILT_PEAKINGEQ;
			else if (strcmp(argv[i], "lowshelf") == 0)
				type = FILT_LOWSHELF;
			else if (strcmp(argv[i], "highshelf") == 0)
				type = FILT_HIGHSHELF;
			else
			{
				fprintf(stderr, "unknown filter type: %s\n",
					argv[i]);
				exit(EXIT_FAILURE);
			}
		}
		else if (strcmp(argv[i], "-help") == 0)
		{
			fprintf(stderr,
				"options: -type [hp|highpass|hpf|lp|lowpass|"
				"lpf|bp_skirt_gain|bp|bandpass|bpf|\nnotch|ap|"
				"allpass|apf|peakingeq|lowshelf|highshelf],\n"
				"-slope arg, -dBgain arg, -q arg, -bw arg,\n"
				"-[cutoff|center|corner|midpoint] arg\n");
			exit(0);
		}
	}

	computecoeffs(&co, type, usewhich, RATE, f0, dBgain, q, bw, slope);
	SET_BINARY_MODE
	filter(&co);
	return 0;
}

static void computecoeffs(biquadcoeffs_t *co, int type, int usewhich,
	float fs, float f0, float dBgain, float q, float bw, float slope)
{
	float A = 0.0f, w0, alpha, beta = 0.0f; /* shut up compiler */
	float a0, a1, a2, b0, b1, b2;

	if (type == FILT_PEAKINGEQ || type == FILT_LOWSHELF
		|| type == FILT_HIGHSHELF)
	{
		A = pow(10, dBgain/40.0f);
	}
	w0 = 2*M_PI * f0 / fs;
	if (usewhich == USE_SLOPE
		&& (type == FILT_LOWSHELF || type == FILT_HIGHSHELF))
	{
		alpha = sin(w0)/2 * sqrt((A + 1/A)*(1/slope - 1) + 2);
	}
	else if (usewhich == USE_BW)
		alpha = sin(w0)*sinh(log(2)/2 * bw * w0/sin(w0));
	else /* usewhich == USE_Q */
		alpha = sin(w0) / (2*q);

	if (type == FILT_LOWSHELF || type == FILT_HIGHSHELF)
		beta = 2*sqrt(A)*alpha;

	switch (type)
	{
	case FILT_LP:
		b0 =  (1 - cos(w0))/2;
		b1 =   1 - cos(w0);
		b2 =  (1 - cos(w0))/2;
		a0 =   1 + alpha;
		a1 =  -2*cos(w0);
		a2 =   1 - alpha;
		break;
	case FILT_HP:
		b0 =  (1 + cos(w0))/2;
		b1 = -(1 + cos(w0));
		b2 =  (1 + cos(w0))/2;
		a0 =   1 + alpha;
		a1 =  -2*cos(w0);
		a2 =   1 - alpha;
		break;
	case FILT_BP_SKIRT_GAIN:
		b0 =  q*alpha; /* or sin(w0)/2 */
		b1 =  0;
		b2 = -q*alpha; /* or -sin(w0)/2 */
		a0 =  1 + alpha;
		a1 = -2*cos(w0);
		a2 =  1 - alpha;
		break;
	case FILT_BP_CONSTANT_PEAK:
		b0 =  alpha;
		b1 =  0;
		b2 = -alpha;
		a0 =  1 + alpha;
		a1 = -2*cos(w0);
		a2 =  1 - alpha;
		break;
	case FILT_NOTCH:
		b0 =  1;
		b1 = -2*cos(w0);
		b2 =  1;
		a0 =  1 + alpha;
		a1 = -2*cos(w0);
		a2 =  1 - alpha;
		break;
	case FILT_AP:
		b0 =  1 - alpha;
		b1 = -2*cos(w0);
		b2 =  1 + alpha;
		a0 =  1 + alpha;
		a1 = -2*cos(w0);
		a2 =  1 - alpha;
		break;
	case FILT_PEAKINGEQ:
		b0 =  1 + alpha*A;
		b1 = -2*cos(w0);
		b2 =  1 - alpha*A;
		a0 =  1 + alpha;
		a1 = -2*cos(w0);
		a2 =  1 - alpha;
		break;
	case FILT_LOWSHELF:
		b0 =    A*((A+1) - (A-1)*cos(w0) + beta);
		b1 =  2*A*((A-1) - (A+1)*cos(w0));
		b2 =    A*((A+1) - (A-1)*cos(w0) - beta);
		a0 =       (A+1) + (A-1)*cos(w0) + beta;
		a1 =   -2*((A-1) + (A+1)*cos(w0));
		a2 =       (A+1) + (A-1)*cos(w0) - beta;
		break;
	case FILT_HIGHSHELF:
		b0 =    A*((A+1) + (A-1)*cos(w0) + beta);
		b1 = -2*A*((A-1) + (A+1)*cos(w0));
		b2 =    A*((A+1) + (A-1)*cos(w0) - beta);
		a0 =       (A+1) - (A-1)*cos(w0) + beta;
		a1 =    2*((A-1) - (A+1)*cos(w0));
		a2 =       (A+1) - (A-1)*cos(w0) - beta;
		break;
	default:
		fprintf(stderr, "invalid filter type\n");
		exit(EXIT_FAILURE);
	}
	co->a0 = a0, co->a1 = a1, co->a2 = a2;
	co->b0 = b0, co->b1 = b1, co->b2 = b2;
}

static void filter(const biquadcoeffs_t *coeffs)
{
	float f;
	int o = 0; /* o = odd */
	float x[2][3] = {{0, 0, 0}, {0, 0, 0}};
	float y[2][3] = {{0, 0, 0}, {0, 0, 0}};
	float c1, c2, c3, c4, c5;
	const int n = 2;

	c1 = coeffs->b0 / coeffs->a0;
	c2 = coeffs->b1 / coeffs->a0;
	c3 = coeffs->b2 / coeffs->a0;
	c4 = coeffs->a1 / coeffs->a0;
	c5 = coeffs->a2 / coeffs->a0;

	while (fread(&f, sizeof f, 1, stdin) == 1)
	{
		o = !o;
		x[o][n-2] = x[o][n-1];
		x[o][n-1] = x[o][n];
		x[o][n] = f;
		y[o][n-2] = y[o][n-1];
		y[o][n-1] = y[o][n];
		y[o][n] = c1*x[o][n] + c2*x[o][n-1] + c3*x[o][n-2]
		                     - c4*y[o][n-1] - c5*y[o][n-2];

		f = y[o][n];

		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			return;
	}
}
