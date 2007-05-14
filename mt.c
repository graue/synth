/*
 * This is an implementation of the Mersenne Twister random number
 * generator. Since I haven't actually read and understood the paper,
 * I just looked at the authors' C code and tried to do what they did,
 * but differently, where I could. The magic numbers and operations
 * performed are the same, though that shouldn't be enough for their
 * copyright to apply.
 *
 * This code is in the public domain; you may deal in it without
 * restriction.
 * Written by Graue <graue@oceanbase.org> on February 23, 2006.
 */

#include <stdint.h>

#define MT_URANDMAX ((uint32_t)0xffffffff)

/*
 * The following parameters dictate the period of the generator.
 *
 * N is the length of the state vector, which is updated all at once
 * each time N random numbers are consumed.
 *
 * Having not actually read the paper, I cannot say exactly what M and
 * VECTOR_A do, but they are used in updating the state vector, in
 * mt_update_state().
 */
#define N        624
#define M        397
#define VECTOR_A 0x9908b0dfUL

static uint32_t state_vector[N];
static int state_index;
static int inited;

void mt_init(uint32_t seed)
{
	state_vector[0] = seed;
	for (state_index = 1; state_index < N; state_index++)
	{
		state_vector[state_index] =
			(1812433253UL * (state_vector[state_index-1]
				^ (state_vector[state_index-1]>>30))
				+ state_index);
		/*
		 * The multiplier here is as described in Knuth's
		 * "The Art of Computer Programming", volume 2, third
		 * edition, page 106.
		 */
	}
	inited = 1;
}

#define MAYBE_A(val) (((val)&1) ? VECTOR_A : 0)
#define UPPER_BIT  0x80000000UL
#define LOWER_BITS 0x7fffffffUL

/*
 * Generate N words.
 * This is called by mt_rand() as necessary.
 */
static void mt_update_state(void)
{
	int ix;
	int num;

	/* Use a default seed if mt_init() has not been called. */
	if (!inited)
		mt_init(5489UL);

	for (ix = 0; ix < N - M; ix++)
	{
		num = (state_vector[ix]&UPPER_BIT)
			| (state_vector[ix+1]&LOWER_BITS);
		state_vector[ix] = state_vector[ix+M] ^ (num>>1)
			^ MAYBE_A(num);
	}

	for (; ix < N-1; ix++)
	{
		num = (state_vector[ix]&UPPER_BIT)
			| (state_vector[ix+1]&LOWER_BITS);
		state_vector[ix] = state_vector[ix+(M-N)] ^ (num>>1)
			^ MAYBE_A(num);
	}

	num = (state_vector[N-1]&UPPER_BIT)
		| (state_vector[0]&LOWER_BITS);
	state_vector[N-1] = state_vector[M-1] ^ (num>>1) ^ MAYBE_A(num);
	state_index = 0;
}

/* Generate a random unsigned 32-bit number. */
uint32_t mt_urand(void)
{
	uint32_t ret;

	/*
	 * Update the state, generating N words at once, when it is
	 * necessary to do so.
	 */
	if (state_index >= N)
		mt_update_state();

	ret = state_vector[state_index++];

	ret ^= (ret>>11);
	ret ^= (ret<<7)  & 0x9d2c5680UL;
	ret ^= (ret<<15) & 0xefc60000UL;
	ret ^= (ret>>18);
	return ret;
}

/* Generate a random nonnegative number, returned as a signed int. */
int32_t mt_rand(void)
{
	return (int32_t)(mt_urand() / 2);
}

/* Generate a random number on [0, 1). */
double mt_frand(void)
{
	double d;

	d = mt_urand();
	d /= ((double)MT_URANDMAX + 1.0);
	return d;
}
