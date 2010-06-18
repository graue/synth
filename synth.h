/* Definitions for the audio used: */

#ifndef DEFAULT_RATE
#define DEFAULT_RATE 44100
#endif

#ifndef LACK_UNISTD_H
#include <unistd.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Useful stuff: */

#ifdef _WIN32
# include <io.h>
# include <fcntl.h>
# define ACTUALLY_SET_BINARY_MODE { \
	setmode(0, O_BINARY); \
	setmode(1, O_BINARY); \
}
#else
# define ACTUALLY_SET_BINARY_MODE ((void)0);
#endif

#ifdef LACK_UNISTD_H
#define SET_BINARY_MODE ACTUALLY_SET_BINARY_MODE
#else
#define SET_BINARY_MODE { \
	ACTUALLY_SET_BINARY_MODE \
	if (isatty(1)) { \
		fprintf(stderr, "stdout should not be a tty\n"); \
		exit(EXIT_FAILURE); \
	} \
}
#endif

#define CLAMP(min, n, max) ((n)<(min)?(min):((n)>(max)?(max):(n)))
