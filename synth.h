/* Definitions for the audio used: */

#ifndef RATE
#define RATE 44100
#endif

/* Useful stuff: */

#ifdef _WIN32
# include <io.h>
# include <fcntl.h>
# define SET_BINARY_MODE { \
	setmode(0, O_BINARY); \
	setmode(1, O_BINARY); \
}
#else
# define SET_BINARY_MODE ((void)0);
#endif

#define CLAMP(min, n, max) ((n)<(min)?(min):((n)>(max)?(max):(n)))
