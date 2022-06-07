#ifndef BIGINT_H_KEQCIY0A
#define BIGINT_H_KEQCIY0A

#include <stdint.h>
#include <string.h>

/* BIGINT bit flags */
/* Error */
#define BIGINT_NAN 	0x1 	/* Not a number 	*/
#define BIGINT_OVF 	0x2 	/* Arithmetic overflow 	*/
#define BIGINT_DIV 	0x4 	/* Division by zero 	*/

#define BIGINT_DIGIT_MASK 	0xff
#define BIGINT_DIGIT_WIDTH 	8 	/* = (sizeof(BIGINT_DIGIT) * CHAR_BIT) */

typedef uint8_t BIGINT_DIGIT;
typedef BIGINT_DIGIT BIGINT;
typedef int BIGINT_INFO;

/* TODO define status codes for BIGIN_INFO */


#if 1
//typedef uint8_t 	BIGINT_WORD;
//typedef BIGINT_WORD 	BIGINT_DIGIT;
//typedef BIGINT_DIGIT 	BIGINT_BUFFER; [> BIGINT_BUFFER serves as syntactic sugar <]
//typedef uint16_t 	BIGINT_DWORD;

#define BIGINT_WORD_MASK 	0xff
#define BIGINT_DWORD_MASK 	0xffff
#define BIGINT_DWORD_MASK_HI 	0xff00
#define BIGINT_DWORD_MASK_LO 	0x00ff
#define BIGINT_SIGN_BIT 	0x80
#define BIGINT_BASE 		256
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

/* Set (copy) src to dest */
BIGINT_INFO bigint_set(BIGINT *const, const char *, const size_t);
//BIGINT *bigint_seti(BIGINT *dest, int n);

/* Add BIGINT 'b' to BIGINT 'a', store the result in 'dest' */
//BIGINT *bigint_add(BIGINT *dest, const BIGINT *a, const BIGINT *b);
//BIGINT *bigint_addi(BIGINT *dest, const BIGINT *a, int n);

/* Subtract 'b' from 'a', store the result in dest */
//BIGINT *bigint_sub(BIGINT *dest, const BIGINT *a, const BIGINT *b);

/* Multiply BIGINT 'a' by BIGINT 'b', the result is
 * stored in 'dest'. */
BIGINT_INFO bigint_mul(BIGINT *, const BIGINT *, const size_t);
BIGINT_INFO bigint_muli(BIGINT *, const int, const size_t);

/* Divide 'dividend' by 'divisor', store the quotient in 'dest', and the
 * remainder in 'rem' */
//BIGINT *bigint_div(BIGINT *dest, const BIGINT *dividend, const BIGINT *divisor, BIGINT *rem);
//BIGINT *bigint_divi(BIGINT *dest, const BIGINT *dividend, const int divisor, int *rem);

/* Base 10 exponential of exp */
//BIGINT *bigint_exp10(BIGINT *dest, const int exp);

BIGINT_INFO bigint_pow(BIGINT *, int, const size_t);

/* string to bigint (helper) */
//int _bigint_stob(BIGINT_BUFFER *buf, size_t size, const char *str);

/* FIXME change the order of these parameters to something sane
 *
 * like (char *dest, size_t dest_size, const BIGINT *a) */
//size_t bigint_tostr(const BIGINT *a, size_t dest_size, char *dest);

void _bigint_dbgprint(BIGINT *a, const size_t);

void bigint_print(const BIGINT *, const size_t);

/* Compute two's complement of 'a' and store the result in 'dest' */
//BIGINT *bigint_complement(BIGINT *dest, const BIGINT *a);

#endif /* end of include guard: BIGINT_H_KEQCIY0A */
