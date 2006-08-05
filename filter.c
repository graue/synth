#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "binary.h"
#include "defs.h"

/* filter.c: highpass, lowpass, or other IIR filter */

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
	float f0 = RATE / 4.0f;
	float q = 1.0f;
	float bw = 1.0f;
	float slope = 1.0f;
	float dBgain = 1.0f;
	int type = FILT_HP;
	int usewhich = USE_Q;
	int i;
	biquadcoeffs_t co;

	SET_BINARY_MODE
	for (i = 1; i < argc; i++)
	{
		if ((strcmp(argv[i], "-cutoff") == NULL
			|| strcmp(argv[i], "-center") == NULL
			|| strcmp(argv[i], "-corner") == NULL
			|| strcmp(argv[i], "-midpoint") == NULL)
			&& ++i < argc)
		{
			f0 = atof(argv[i]);
			f0 = CLAMP(1.0f, f0, RATE * 0.499f);
		}
		else if ((strcmp(argv[i], "-dbgain") == NULL
			|| strcmp(argv[i], "-dBgain") == NULL)
			&& ++i < argc)
		{
			dBgain = atof(argv[i]);
			dBgain = CLAMP(-100.0f, q, 100.0f);
		}
		else if (strcmp(argv[i], "-q") == NULL && ++i < argc)
		{
			q = atof(argv[i]);
			q = CLAMP(0.01f, q, 100.0f);
			usewhich = USE_Q;
		}
		else if (strcmp(argv[i], "-bw") == NULL && ++i < argc)
		{
			bw = atof(argv[i]);
			bw = CLAMP(0.01f, bw, 100.0f);
			usewhich = USE_BW;
		}
		else if (strcmp(argv[i], "-slope") == NULL && ++i < argc)
		{
			slope = atof(argv[i]);
			slope = CLAMP(0.01f, slope, 100.0f);
			usewhich = USE_SLOPE;
		}
		else if (strcmp(argv[i], "-type") == NULL && ++i < argc)
		{
			if (strcmp(argv[i], "hp") == NULL
				|| strcmp(argv[i], "highpass") == NULL
				|| strcmp(argv[i], "hpf") == NULL)
			{
				type = FILT_HP;
			}
			else if (strcmp(argv[i], "lp") == NULL
				|| strcmp(argv[i], "lowpass") == NULL
				|| strcmp(argv[i], "lpf") == NULL)
			{
				type = FILT_LP;
			}
			else if (strcmp(argv[i], "bp_skirt_gain") == NULL)
				type = FILT_BP_SKIRT_GAIN;
			else if (strcmp(argv[i], "bp") == NULL
				|| strcmp(argv[i], "bandpass") == NULL
				|| strcmp(argv[i], "bpf") == NULL)
			{
				type = FILT_BP_CONSTANT_PEAK;
			}
			else if (strcmp(argv[i], "notch") == NULL)
				type = FILT_NOTCH;
			else if (strcmp(argv[i], "ap") == NULL
				|| strcmp(argv[i], "allpass") == NULL
				|| strcmp(argv[i], "apf") == NULL)
			{
				type = FILT_AP;
			}
			else if (strcmp(argv[i], "peakingeq") == NULL)
				type = FILT_PEAKINGEQ;
			else if (strcmp(argv[i], "lowshelf") == NULL)
				type = FILT_LOWSHELF;
			else if (strcmp(argv[i], "highshelf") == NULL)
				type = FILT_HIGHSHELF;
			else
			{
				fprintf(stderr, "unknown filter type: %s\n",
					argv[i]);
				exit(EXIT_FAILURE);
			}
		}
	}

	computecoeffs(&co, type, usewhich, RATE, f0, dBgain, q, bw, slope);
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
	float b0, b1, b2, a0, a1, a2;
	float x[2][3] = {{0, 0, 0}, {0, 0, 0}};
	float y[2][3] = {{0, 0, 0}, {0, 0, 0}};
	const int n = 2;

	b0 = coeffs->b0, b1 = coeffs->b1, b2 = coeffs->b2;
	a0 = coeffs->a0, a1 = coeffs->a1, a2 = coeffs->a2;

	for (;;)
	{
		if (fread(&f, sizeof f, 1, stdin) < 1)
			return;

		o = !o;
		x[o][n-2] = x[o][n-1];
		x[o][n-1] = x[o][n];
		x[o][n] = f;
		y[o][n-2] = y[o][n-1];
		y[o][n-1] = y[o][n];
		y[o][n] = (b0/a0)*x[o][n] + (b1/a0)*x[o][n-1] + (b2/a0)*x[o][n-2]
		                          - (a1/a0)*y[o][n-1] - (a2/a0)*y[o][n-2];

		f = y[o][n];

		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			return;
	}
}
