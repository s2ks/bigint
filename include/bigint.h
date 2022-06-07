#ifndef BIGINT_H_KEQCIY0A
#define BIGINT_H_KEQCIY0A

#include <stdint.h>
#include <string.h>

/* BIGINT bit flags */
/* Error */
#define BIGINT_NAN 	0x1 	/* Not a number 	*/
#define BIGINT_OVF 	0x2 	/* Arithmetic overflow 	*/
#define BIGINT_DIV 	0x4 	/* Division by zero 	*/
/* Status */
#define BIGINT_UNSIGNED 0x100 	/* Treat as unsigned 	*/

#define BIGINT_STRING_INIT(BUF, BUF_SIZE, STR) (BIGINT) {.buf = BUF, .size = SIZE, \
	.flag = bigint_strtobig(&(BIGINT) {.buf = BUF, .size = SIZE}, STR)->flag}

#define BIGINT_ZERO_INIT(BUF, SIZE) (BIGINT) {.buf = BUF, .size = SIZE, .flag = 0}; \
	memset(BUF, 0, SIZE)

#define BIGINT_INIT(BUF, SIZE, VAL) (BIGINT) {.buf = BUF, .size = SIZE, \
	.flag = bigint_seti(&(BIGINT) {.buf = BUF, .size = SIZE}, VAL)->flag}

#define BIGINT_ASSIGN(BUF, SIZE) (BIGINT) {.buf = BUF, .size = SIZE, .flag = 0}
#define BIGINT_ASSIGN_UNSIGNED(BUF, SIZE) (BIGINT) {.buf = BUF, .size = SIZE, \
	.flag = BIGINT_UNSIGNED}

/* BIGINT zero constant, useful for comparison */
/* FIXME consider defining an array with a bunch of useful BIGINT constants */
#define BIGINT_ZERO (BIGINT) {.buf = (BIGINT_BUFFER[1]) {0}, \
	.size = sizeof(BIGINT_BUFFER), .flag = 0}

/* Compute the maximum capacity (in BIGINT_DIGITs) of BIGINT,
 * pointed to by BPTR. */
#define BIGINT_CAP(BPTR) ((BPTR)->size / sizeof(BIGINT_BUFFER))


typedef uint8_t BIGINT;
typedef int BIGINT_INFO;

/* TODO define status codes for BIGIN_INFO */


#if 1
//typedef uint8_t 	BIGINT_WORD;
//typedef BIGINT_WORD 	BIGINT_DIGIT;
//typedef BIGINT_DIGIT 	BIGINT_BUFFER; [> BIGINT_BUFFER serves as syntactic sugar <]
//typedef uint16_t 	BIGINT_DWORD;

#define BIGINT_DIGIT_MASK 	0xff
#define BIGINT_WORD_MASK 	0xff
#define BIGINT_DWORD_MASK 	0xffff
#define BIGINT_DWORD_MASK_HI 	0xff00
#define BIGINT_DWORD_MASK_LO 	0x00ff
#define BIGINT_SIGN_BIT 	0x80
#define BIGINT_BASE 		256
#define BIGINT_DIGIT_WIDTH 	8 	/* = (sizeof(BIGINT_DIGIT) * CHAR_BIT) */
#define BIGINT_WORD_WIDTH 	8
#else
typedef uint32_t BIGINT_DIGIT;
typedef BIGINT_DIGIT BIGINT_BUFFER;

#define BIGINT_SIGN_BIT 	0x80000000
#define BIGINT_BASE 		0x100000000ULL
#define BIGINT_DIGIT_WIDTH 	32
#endif

#define BIGINT_LENTOSIZE(L) (L * sizeof(BIGINT_DIGIT))
#define BIGINT_SIZETOLEN(S) (MAX(S / sizeof(BIGINT_DIGIT), 1))

#define BIGINT_ISNEGATIVE(BPTR) ((BPTR)->buf[BIGINT_CAP(BPTR) - 1] & BIGINT_SIGN_BIT)
#define BIGINT_ISUNSIGNED(BPTR) ((BPTR)->flag & BIGINT_UNSIGNED)

/* FIXME to speed up carries, maybe we can keep track of indices where we should
 * put the next carry bit? This is a very rough idea, but there is probably
 * a way to use more memory (we have plenty) to offload the cpu (we're incessantly
 * limited).
 * Consider using something similar to a dp table. This isn't limited to
 * just carrying, we can also apply this to other parts of common operations
 * we have to do to compute BIGINT values. */

/* TODO usage should be
 * BIGINT b[n] where n is the size of the integer in bytes internally we can convert it
 * to an array of larger integer if alignment allows it. */
//typedef struct {
	//BIGINT_DIGIT 	*buf; 	[> A BIGINT is an array of BIGINT_DIGITs <]
	//size_t 		size; 	[> The size of buf in bytes <]

	//uint32_t 	flag; 	[> Bit flags specifying the state of BIGINT <]

	//[>size_t 	digits;<] /* TODO use this to keep track of the number of
				   //* (active) digits */

//[> 	<some_type> 	*carry_table; TODO <]
//} BIGINT;

/* Set (copy) src to dest */
BIGINT *bigint_set(BIGINT *dest, const BIGINT *src);
BIGINT *bigint_seti(BIGINT *dest, int n);

/* Add BIGINT 'b' to BIGINT 'a', store the result in 'dest' */
BIGINT *bigint_add(BIGINT *dest, const BIGINT *a, const BIGINT *b);
BIGINT *bigint_addi(BIGINT *dest, const BIGINT *a, int n);

/* Subtract 'b' from 'a', store the result in dest */
BIGINT *bigint_sub(BIGINT *dest, const BIGINT *a, const BIGINT *b);

/* Multiply BIGINT 'a' by BIGINT 'b', the result is
 * stored in 'dest'. */
BIGINT *bigint_long_mul(BIGINT *dest, const BIGINT *a, const BIGINT *b);
BIGINT *bigint_mul(BIGINT *dest, const BIGINT *a, const BIGINT *b);
BIGINT *bigint_muli(BIGINT *dest, const BIGINT *a, const int n);

/* Divide 'dividend' by 'divisor', store the quotient in 'dest', and the
 * remainder in 'rem' */
BIGINT *bigint_div(BIGINT *dest, const BIGINT *dividend, const BIGINT *divisor, BIGINT *rem);
BIGINT *bigint_divi(BIGINT *dest, const BIGINT *dividend, const int divisor, int *rem);

/* Base 10 exponential of exp */
BIGINT *bigint_exp10(BIGINT *dest, const int exp);

#if 0
/* FIXME we no longer use repeated multiplication */
/* Raise BIGINT 'a', to the power of 'p' by method of repeated multiplication,
 * store the result in 'dest' */
BIGINT *bigint_pow(BIGINT *dest, const BIGINT *a, const int p);

/* FIXME this method has been reimplemented using iteration rather
 * than recursion, and renamed to bigint_pow */
/* Raise 'a' by the power of 'p' by method of exponentiation by squaring,
 * store the result in 'dest' */
BIGINT *bigint_pow2(BIGINT *dest, const BIGINT *a, const int p);
#endif

BIGINT *bigint_pow(BIGINT *dest, const BIGINT *a, intmax_t p);

/* string to bigint (helper) */
//int _bigint_stob(BIGINT_BUFFER *buf, size_t size, const char *str);

/* FIXME change the order of these parameters to something sane
 *
 * like (char *dest, size_t dest_size, const BIGINT *a) */
size_t bigint_tostr(const BIGINT *a, size_t dest_size, char *dest);

void _bigint_dbgprint(BIGINT *a);

/* Compute two's complement of 'a' and store the result in 'dest' */
BIGINT *bigint_complement(BIGINT *dest, const BIGINT *a);

#endif /* end of include guard: BIGINT_H_KEQCIY0A */
