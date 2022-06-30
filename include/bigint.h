#ifndef BIGINT_H_KEQCIY0A
#define BIGINT_H_KEQCIY0A

#include <stdint.h>
#include <string.h>

#ifndef BIGINT_DEBUG_OUT
#define BIGINT_DEBUG_OUT stdout
#endif

/* BIGINT_INFO bit flags */
/* Error */
#define BIGINT_NAN 	1 << 0 	/* Not a number 	*/
#define BIGINT_OVF 	1 << 1 	/* Arithmetic overflow 	*/
#define BIGINT_DIV 	1 << 2	/* Division by zero 	*/

/* Warning/status */
#define BIGINT_MOSTNEG 	1 << 10 /* Most negative value */

#define BIGINT_DIGIT_MASK 	0xff
#define BIGINT_DIGIT_WIDTH 	8 	/* = (sizeof(BIGINT_DIGIT) * CHAR_BIT) */
#define BIGINT_SIGN_BIT 	0x80

typedef uint8_t BIGINT_DIGIT;
typedef BIGINT_DIGIT BIGINT;
typedef int BIGINT_INFO;

/* TODO define BIGINT_WORD, DWORD and QWORD types */

#define BIGINT_WORD_MASK 	0xff
#define BIGINT_DWORD_MASK 	0xffff
#define BIGINT_DWORD_MASK_HI 	0xff00
#define BIGINT_DWORD_MASK_LO 	0x00ff
#define BIGINT_BASE 		256
#define BIGINT_WORD_WIDTH 	8

#define BIGINT_ISNEGATIVE(A, SIZE) !!(A[SIZE - 1] & BIGINT_SIGN_BIT)

/* Maximum length of a base 10 string conversion, including an eventual '-' character and the null terminator */
/* The number of base 10 digits needed to represent a base 256 number
 * turns out to trend toward the number of base 256 digits multiplied by log(256)
 *
 * log(256) = ~2.4082400798797607421875 */
#define BIGINT_MAXSTRLEN10(SIZE) ((size_t) (((float) SIZE * 2.4082400798797607421875f) + 1.0f)) + 2

/* Same as BIGINT_MAXSTRLEN10 but for base 16 and including the hexadecimal prefix "0x..." */
#define BIGINT_MAXSTRLEN16(SIZE) (SIZE * 2) + 4

/* Set a BIGINT of length 'size' to the value represented by the string 'val' */
BIGINT_INFO bigint_set(BIGINT *const a, const char *val, const size_t size);
//BIGINT *bigint_seti(BIGINT *dest, int n);

/* Shift BIGINT 'a' of length 'size', to the left by a whole digit's width 'n' times */
BIGINT_INFO bigint_dshl(BIGINT *const a, const size_t n, const size_t size);
/* Shift BIGINT 'a' of length 'size', to the right by a whole digit's width 'n' times */
BIGINT_INFO bigint_dshr(BIGINT *const a, const size_t n, const size_t size);

/* Add BIGINT 'b' to BIGINT 'a', store the result in 'a' */
BIGINT_INFO bigint_add(BIGINT *const a, const BIGINT *const b, const size_t size);
BIGINT_INFO bigint_addi(BIGINT *const a, const unsigned n, const size_t size);

/* Subtract 'b' from 'a', store the result in 'a' */
BIGINT_INFO bigint_sub(BIGINT *const a, const BIGINT *const b, const size_t size);

/* Multiply BIGINT 'a' by BIGINT 'b', the result is
 * stored in 'a'. */
BIGINT_INFO bigint_mul(BIGINT *const a, BIGINT *b, const size_t size);
BIGINT_INFO bigint_muli(BIGINT *const a, const int b, const size_t size);

/* Divide 'dividend' by 'divisor', store the quotient in 'a', and the
 * remainder in 'rem'. 'rem' is optional and can be NULL */
//BIGINT *bigint_div(BIGINT *dest, const BIGINT *dividend, const BIGINT *divisor, BIGINT *rem);
BIGINT_INFO bigint_div(BIGINT *const a, BIGINT *const b, BIGINT *const rem, const size_t size);
BIGINT_INFO bigint_divi(BIGINT *a, const int b, int *const rem, const size_t size);

/* Base 10 exponential of exp */
//BIGINT *bigint_exp10(BIGINT *dest, const int exp);

/* Raise BIGINT 'a' of size 'size' to the pth power */
BIGINT_INFO bigint_pow(BIGINT *const a, int p, const size_t size);

/* Convert /a/ to a textual base 10 representation and write to /dest/. Note that /dest/ is assumed to have
 * a size of at least BIGINT_MAXSTRLEN10(size) */
size_t bigint_tostr(char *const dest, const BIGINT *const a, const size_t size);

/* Print a base 10 string representation of 'a' */
void bigint_print(const BIGINT *const a, const size_t size);

/* Print the raw bytes of 'a' to BIGINT_DEBUG_OUT (stdout by default) */
void bigint_printraw(const BIGINT *const a, const size_t size);

/* Compute two's complement of 'a' and store the result in 'dest' */
BIGINT_INFO bigint_complement(BIGINT *const a, const size_t size);

#endif /* end of include guard: BIGINT_H_KEQCIY0A */
