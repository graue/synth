// fade for float samples
// invocation:
// fadef in|out lin|log|cos|logcos start end [quietvol]

// with lin, cos: quietvol is a multiplier (0 < quietvol < 1)
// with log, logcos: quietvol is a number of dB (quietvol < 0)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <err.h>
#include <sys/types.h>
#include "synth.h"
#include "rate.inc"

enum { LIN, LOG, COS, LOGCOS };

#define M_20_OVER_LN10 8.68588963806503655302257838
#define M_LN10_OVER_20 0.115129254649702284200899573
#define RATTODB(x) (log(x) * M_20_OVER_LN10)
#define DBTORAT(x) exp((x) * M_LN10_OVER_20)

static void usage(void)
{
	errx(1, "usage: fadef in|out lin|log|cos|logcos start end [quietvol]");
}

int main(int argc, char *argv[])
{
	int fostart, foend;
	int fonum = 0;
	int fadetype = LIN;
	int fadein = 0;
	float loudvol, quietvol;
	float smp[2];
	float fofstart, fofend;

	if (argc != 5 && argc != 6)
		usage();

	if (!strcasecmp(argv[1], "in"))
		fadein = 1;
	else if (!strcasecmp(argv[1], "out"))
		fadein = 0;
	else usage();

	if (!strcasecmp(argv[2], "lin"))
		fadetype = LIN;
	else if (!strcasecmp(argv[2], "log"))
		fadetype = LOG;
	else if (!strcasecmp(argv[2], "cos"))
		fadetype = COS;
	else if (!strcasecmp(argv[2], "logcos"))
		fadetype = LOGCOS;
	else usage();

	fofstart = atof(argv[3]);
	fofend = atof(argv[4]);

	if (fadetype == LIN || fadetype == COS)
	{
		loudvol = 1.0;
		quietvol = 0.0;
	}
	else // LOG, LOGCOS
	{
		loudvol = 0.0; // dB
		quietvol = -90.0; // dB
	}
	if (argc == 6) // custom quietvol supplied
		quietvol = atof(argv[5]);

	fostart = (int)(fofstart * RATE);
	foend = (int)(fofend * RATE);

	// before the fade
	while (fostart > 0 && fread(smp, sizeof smp[0], 2, stdin) == 2)
	{
		fostart--, foend--;
		if (fwrite(smp, sizeof smp[0], 2, stdout) < 2)
			err(1, "write error");
	}

	// during the fade
	while (fonum < foend && fread(smp, sizeof smp[0], 2, stdin) == 2)
	{
		double progress;
		double amp;

		progress = (double)fonum / foend;

		if (fadein)
			progress = 1.0 - progress;

		if (fadetype == COS || fadetype == LOGCOS)
			progress = 1.0 - cos(progress * M_PI/2);

		amp = loudvol*(1.0 - progress) + quietvol*progress;

		if (fadetype == LOG || fadetype == LOGCOS)
			amp = DBTORAT(amp);

		smp[0] *= amp;
		smp[1] *= amp;

		if (fwrite(smp, sizeof smp[0], 2, stdout) < 2)
			err(1, "write error");

		fonum++;
	}

	// after the fade
	while (fread(smp, sizeof smp[0], 2, stdin) == 2)
	{
		if (fwrite(smp, sizeof smp[0], 2, stdout) < 2)
			err(1, "write error");
	}

	return 0;
}
