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
