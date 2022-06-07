#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define __DEBUG_OUT stdout

#include "bigint.h"


void _bigint_dbgprint(BIGINT *a, size_t size) {
	const int cap = size;

	for(int i = 0; i < cap; i++) {
		fprintf(__DEBUG_OUT, "%02x ", a[i]);
	}
	fprintf(__DEBUG_OUT, "\n");
}

size_t bigint_digits(const BIGINT *a, const size_t size) {
	if(a == NULL) {
		return 0;
	}

	for(size_t i = size; i > 0; i--) {
		if(a[i - 1] != 0) {
			return i;
		}
	}

	return 0;
}

/* Ripple carry (?)
 *
 * Carry the value of 'n' to position 'pos' in BIGINT 'a'. */

/* TODO Optimise this */
int bigint_carry(BIGINT *a, unsigned carry, size_t pos, const size_t size) {
	int 	accum;
	BIGINT_INFO 	info = 0;

	do {
		/* Index out of range -> Overflow */
		if(pos >= size) {
			info |= BIGINT_OVF;
			return info;
		}

		accum = a[pos] + carry;

		/* carry = accum / BIGINT_BASE */
		carry = accum >> BIGINT_DIGIT_WIDTH;

		/* accum -= carry * BIGINT_BASE */
		accum -= carry << BIGINT_DIGIT_WIDTH;

		a[pos++] = accum;
	} while(carry > 0);

	return info;
}


#if 0
/* Bit shifts */

/* Shift left */
/* TODO implement */
BIGINT *bigint_shl(BIGINT *t, size_t n, size_t size) {
	((void) t);
	((void) n);

	return NULL;
}

/* Shift right */
/* TODO implement */
BIGINT *bigint_shr(BIGINT *t, size_t n, size_t size) {
	((void) t);
	((void) n);

	return NULL;
}

/* BIGINT to int conversion
 *
 * read: b.to.i (NOT BOYTOY) */
/* FIXME new usage */
int bigint_btoi(const BIGINT *b, size_t size) {
	int res = 0;
	const size_t reslen 	= BIGINT_SIZETOLEN(sizeof(res));
	const size_t blen 	= bigint_digits(b, size);

	for(size_t i = MIN(blen, reslen); i > 0 ; i--) {
		res <<= BIGINT_DIGIT_WIDTH;
		res |= b->buf[i - 1];
	}

	return res;
}


/* FIXME new usage */
BIGINT *bigint_itob(BIGINT *dest, int n) {
	const size_t nlen = BIGINT_SIZETOLEN(sizeof(n));
	const size_t dcap = BIGINT_CAP(dest);

	for(size_t i = 0; i < MIN(nlen, dcap); i++) {
		dest->buf[i] = (n & BIGINT_DIGIT_MASK);
		n >>= BIGINT_DIGIT_WIDTH;
	}

	return dest;
}

/* Digit shift, (or dash?) shift by 'n' number
 * of BIGINT_DIGITs */

/* Digit shift left
 * Shift 't' by 'n' BIGINT_DIGITs to the left */
BIGINT *bigint_dshl(BIGINT *t, size_t n) {
	const size_t len = bigint_digits(t);
	const size_t cap = BIGINT_CAP(t);

	for(size_t i = len; i > 0; i--) {
		/* Carry the current digit to position i + n
		 *
		 * Make sure we don't write out of bounds,
		 * if we would be writing out of bounds we set
		 * the overflow flag. */
		if((i + n - 1) < cap) {
			t->buf[i + n - 1] = t->buf[i - 1];
		} else {
			t->flag |= BIGINT_OVF;
		}
		t->buf[i - 1] = 0;
	}

	return t;
}

/* Digit shift right by 'n' BIGINT_DIGITs */
BIGINT *digit_dshr(BIGINT *t, size_t n) {
	const size_t len = bigint_digits(t);
	const size_t cap = BIGINT_CAP(t);

	for(size_t i = 0; i < len; i--) {
		if(n <= i) {
			t->buf[i - n] = t->buf[i];
		}

		/* If the sign bit is set we need to make
		 * sure the sign bit is preserved. */
		if(t->buf[cap - 1] & BIGINT_SIGN_BIT) {
			t->buf[i] = ~(0);
		} else {
			t->buf[i] = 0;
		}
	}

	return t;
}

/* Compute two's complement of 'a' */
/* NOTE: Be mindful of most negative value */
BIGINT *bigint_complement(BIGINT *dest, const BIGINT *a) {
	const size_t cap = BIGINT_CAP(dest);

	bigint_set(dest, a);

	/* We find the two's complement of a number
	 * by inverting all bits and adding one */

	/* Invert all bits */
	for(size_t i = 0; i < cap; i++) {
		dest->buf[i] = ~(dest->buf[i]);
	}

	/* Add one */
	bigint_carry(dest, 0, 1);
	/*bigint_addi(dest, dest, 1);*/

	return dest;
}

/* Compare a to b
 *
 * a > b -> return 1
 * a < b -> return -1
 * a = b -> return 0 */

/* FIXME new usage */
int bigint_cmp(const BIGINT *const a, const BIGINT *const b) {
	const size_t alen = bigint_digits(a);
	const size_t blen = bigint_digits(b);

	int cmp = 0;

	const size_t len = MAX(alen, blen);

	for(size_t i = len; i > 0; i--) {
		BIGINT_BUFFER digit_a = alen >= i ? a->buf[i - 1] : 0;
		BIGINT_BUFFER digit_b = blen >= i ? b->buf[i - 1] : 0;

		if(digit_a > digit_b) {
			cmp = 1;
			break;
		} else if(digit_a < digit_b) {
			cmp = -1;
			break;
		}
	}

	/* Flip the compare result if either a is negative
	 * or b is negative, but not both */
	if(BIGINT_ISNEGATIVE(a) ^ BIGINT_ISNEGATIVE(b)) {
		cmp = -cmp;
	}

	return cmp;
}
#endif

#if 0
/* FIXME what if we set a negative? Make sure it actually does
 * what you think it does */
BIGINT *bigint_set(BIGINT *dest, const BIGINT *src) {

	/* If src points to the same structure as
	 * dest then there is nothing to set. */
	if(dest == src) {
		return dest;
	}

	const size_t dest_cap 	= BIGINT_CAP(dest);
	const size_t src_cap 	= BIGINT_CAP(src);
	const size_t src_digits	= bigint_digits(src);

	/* It is not impossible for src and dest to share
	 * the same buffer, we must make sure src is
	 * never affected. */

	/* FIXME how does this solve anything? */
	/* FIXME description:
	 * It is possible src and dest share the same
	 * buffer, or parts of the same buffer.
	 * We do not want to memset the destination buffer directly
	 * because we might lose parts of the source buffer before
	 * we get to the copying part. */
	BIGINT_BUFFER store_buf[BIGINT_CAP(dest)];

	/* Set the overflow flag if if the number of
	 * digits in src exceeds the capacity of dest */
	if(src_digits > dest_cap) {
		dest->flag |= BIGINT_OVF;
	}

	/* If the sign bit is set we may need to sign extend (unless the UNSIGNED
	 * flag is set) because dest might be larger than src */
	if(src->buf[src_cap - 1] & BIGINT_SIGN_BIT && (src->flag & BIGINT_UNSIGNED) == 0) {
		memset(store_buf, 0xff, dest->size);
	} else {
		memset(store_buf, 0, dest->size);
	}

	/* Set all digits of src individually in dest
	 * as long as i does not exceed the capacity
	 * of dest */
	for(size_t i = 0; i < src_digits && i < dest_cap; i++) {
		store_buf[i] = src->buf[i];
	}

	memcpy(dest->buf, store_buf, dest->size);

	return dest;
}
#endif

#if 0
BIGINT_INFO bigint_seti(BIGINT *dest, int n) {

	/* Initialise to zero */
	memset(dest->buf, 0, dest->size);

	/* if 'n' is negative we carry the absolute value of 'n'
	 * and compute the complement. */
	if(n < 0) {
		bigint_carry(dest, 0, (unsigned) (-n));
		bigint_complement(dest, dest);
	} else {
		bigint_carry(dest, 0, (unsigned) n);
	}

	return dest;
}
#endif

/* 'val' should be a string representing a base 10 number, mathematical operators (like +, -, *, /, ^) are not allowed.
 * However the number can be written in scientific notation, e.g. 25e100 for 25 * 10^100 */
BIGINT_INFO bigint_set(BIGINT *const dest, const char *val, const size_t size) {
	BIGINT_INFO info = 0;

	memset(dest, 0, size);

	while(*val != '\0' && *val != 'e') {
		if(isdigit(*val) == 0) {
			/* TODO Invalid character set a flag and handle */
			info |= BIGINT_NAN;
			break;
		}

		info |= bigint_muli(dest, 10, size);

		BIGINT_DIGIT digit = (BIGINT_DIGIT) (*val - '0');

		printf("digit: %d\n", digit);

		info |= bigint_carry(dest, digit, 0, size);
		_bigint_dbgprint(dest, size);
		val++;
	}

	if(*val == 'e') {
		/* Raise 10 to the power of what comes after 'e'. The power should be able to be
		 * converted to a regular integer, this should cover a large enough range of numbers we
		 * could ever want to use */
		BIGINT b[size];
		memset(b, 0, size);

		b[0] = 10;

		info |= bigint_pow(b, atoi(++val), sizeof(b));
		info |= bigint_mul(dest, b, size);
	}

	return info;
}

void bigint_print(const BIGINT *a, const size_t size) {
	
}

#if 0
/* Call bigint_tostr with size zero and dest NULL to get the size
 * of the computed string
 *
 * char *dest_buf[bigint_tostr(&my_bigint, 0, NULL)];
 * bigint_tostr(&my_bigint, sizeof(dest_buf), dest_buf); */

/* TODO There are a lot of performance improvements that can be made
 * here */
size_t bigint_tostr(const BIGINT *a, size_t dest_size, char *dest) {
	size_t digitc = bigint_digits(a);
	size_t unwritten = 0;

	BIGINT_BUFFER store_buf[BIGINT_CAP(a)];
	BIGINT store = BIGINT_ASSIGN(store_buf, sizeof(store_buf));

	const int radix = 1e9;

	memcpy(store_buf, a->buf, sizeof(store_buf));

	/* Reserve a space for the null terminator */
	if(dest && dest_size > 0) {
		dest[dest_size - 1] = '\0';
	} else {
		unwritten++;
	}

	size_t i = dest_size > 0 ? dest_size - 1 : 0;

	/* FIXME This is super slow for very large numbers.
	 * Find a way to compute the number of digits that would be involved
	 * if represented as a base 10 number.
	 *
	 * We could also replace the comparison to zero, with
	 * a digit count, where we remember the top of the last digit
	 * from the last iteration */
	while(digitc > 0) {
		int rem;

		/* FIXME also define a BIGINT constant for
		 * radix so we don't have to do conversion on
		 * each iteration */
		bigint_divi(&store, &store, radix, &rem);

		/* FIXME leading zeros */
		for(int j = radix / 10; j > 0; j /= 10) {
			if(dest && i > 0) {
				dest[--i] = (rem % 10) + '0';
			} else {
				unwritten++;
			}

			rem /= 10;
		}

		while(store.buf[digitc - 1] == 0 && digitc > 0) {
			digitc--;
		}
	}

	/* We write the characters from back-to-front in the buffer,
	 * this is not a problem if the buffer is exactly the
	 * same size as the number of characters to be written.
	 * But if the destination buffer is larger than the number
	 * of characters we write we need to shuffle the characters
	 * to the front of the buffer.
	 *
	 * XXXXXX1234 -> 1234XXXXXX (of course we include the null terminator
	 * in this too) */

	/* if i == 0 we don't need to go through this song-and-dance */
	if(dest_size > 0 && dest && i > 0) {
		const size_t len = dest_size - i;
		for(size_t x = 0; x < len; x++, i++) {
			dest[x] = dest[i];
		}
	}

	return unwritten;
}
#endif
