#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "bigint.h"

void bigint_printraw(const BIGINT *const a, const size_t size) {

	for(size_t i = 0; i < size; i++) {
		fprintf(BIGINT_DEBUG_OUT, "%02x ", a[i]);
	}
	fprintf(BIGINT_DEBUG_OUT, "\n");
}

size_t bigint_digits(const BIGINT *const a, const size_t size) {
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
int bigint_carry(BIGINT *const a, unsigned carry, size_t pos, const size_t size) {
	unsigned 	accum;
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
#endif

/* Digit shift, (or dash?) shift by 'n' number
 * of BIGINT_DIGITs */

/* Digit shift left
 * Shift 't' by 'n' BIGINT_DIGITs to the left */
BIGINT_INFO bigint_dshl(BIGINT *const a, const size_t n, const size_t size) {
	const size_t len = bigint_digits(a, size);
	const size_t cap = size;
	BIGINT_INFO info = 0;

	/* TODO could we shift the digits by simply doing the following?
	 * 	memcpy(a + n, a, size - n);
	 * 	memset(a, 0, n)
	 *
	 * NOTE: from the man page: the areas of memory must not overlap
	 * when using memcpy, use memmove instead. */

	for(size_t i = len; i > 0; i--) {
		/* Carry the current digit to position i + n
		 *
		 * Make sure we don't write out of bounds,
		 * if we would be writing out of bounds we set
		 * the overflow flag. */
		if((i + n - 1) < cap) {
			a[i + n - 1] = a[i - 1];
		} else {
			info |= BIGINT_OVF;
		}
		a[i - 1] = 0;
	}
	return info;
}

#if 0
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
#endif

#if 0
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

/* TODO validate a string representing a number.
 * See regex.h perhaps? */
int bigint_validate_str(const char *const str) {
	return 1;
}

/* 'val' should be a string representing a base 10 number, mathematical operators (like +, -, *, /, ^) are not allowed.
 * However the number can be written in scientific notation, e.g. 25e100 for 25 * 10^100 */
/* TODO support hexadecimal -> use toupper and then isxdigit.
 * if c <= '9' then use c - '0' otherwise use c - 'A' + 10*/
/* TODO support setting negatives */
/* TODO support decimal places in scientific notation e.g. 4.5e101 = 45e100 */
BIGINT_INFO bigint_set(BIGINT *const dest, const char *val, const size_t size) {
	BIGINT_INFO info = 0;

	if(bigint_validate_str(val) == 0) {
		/* TODO Invalid string */
	}

	memset(dest, 0, size);

	while(*val != '\0' && *val != 'e') {
		if(isdigit(*val) == 0) {
			/* TODO Invalid character set a flag and handle */
			info |= BIGINT_NAN;
			break;
		}

		info |= bigint_muli(dest, 10, size);

		BIGINT_DIGIT digit = (BIGINT_DIGIT) (*val - '0');

		info |= bigint_carry(dest, digit, 0, size);
		val++;
	}

	if(*val == 'e') {
		/* Raise 10 to the power of what comes after 'e'. The power should be able to be
		 * converted to a regular integer, this should cover a large enough range of numbers we
		 * could ever want to use */

		/* TODO is there a way to perform this operation without allocating temporary storage space?
		 * We just need to multiply by 10 a bunch of times */
		BIGINT *b = malloc(size);
		memset(b, 0, size);

		b[0] = 10;

		info |= bigint_pow(b, atoi(++val), size);
		info |= bigint_mul(dest, b, size);

		free(b);
	}

	return info;
}

void bigint_print(const BIGINT *const a, const size_t size) {
	char buf[BIGINT_MAXSTRLEN10(size)];

	bigint_tostr(buf, a, size);

	printf("%s\n", buf);
}

/* Convert /a/ to a base 10 string representation, returns the number of bytes
 * written to /d/ */
/* TODO use a larger radix e.g. 1e9 */
size_t bigint_tostr(char *const d, const BIGINT *const a, const size_t size) {
	char *dest = d;
	size_t w = 0;

	BIGINT n[size];

	memcpy(n, a, size);

	if(BIGINT_ISNEGATIVE(a, size)) {
		bigint_complement(n, size);
	}


	/* By taking the remainder of a repeated division of /n/ we get the base 10 digits
	 * of /n/. As a side effect we write the digits to /dest/ in reverse order... */
	for(size_t ds = bigint_digits(n, size); ds > 0; ds--) {
		const size_t i = ds - 1;
		int rem;

		while(n[i] > 0) {
			bigint_divi(n, 10, &rem, ds);
			*(dest++) = '0' + rem;
			w++;
		}
	}

	if(BIGINT_ISNEGATIVE(a, size)) {
		*(dest++) = '-';
		w++;
	}

	*dest = '\0';
	w++;

	dest = d;

	/* ...So reverse the order of the characters */
	for(size_t b = 0, t = strlen(dest) - 1; b < t; b++, t--) {
		const char top = dest[t];
		const char bot = dest[b];

		dest[b] = top;
		dest[t] = bot;
	}

	return w;
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
