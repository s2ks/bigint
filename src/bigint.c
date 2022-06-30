#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#include "bigint.h"

#include <stdio.h>

/* TODO int to bigint and bigint to int conversion helpers */

size_t bigint_digits(const BIGINT *, const size_t);
BIGINT_INFO bigint_carry(const BIGINT *, unsigned, size_t, const size_t);

enum {
	ZERO_VAL,
	TEN_VAL,

	/* --- */
	CONST_COUNT
};

const BIGINT *bigint_const[CONST_COUNT] = {
	[ZERO_VAL] 	= (BIGINT[1]) {0},
	[TEN_VAL] 	= (BIGINT[1]) {10},
};


/* Compute two's complement of 'a' */
/* NOTE: Be mindful of most negative value */
BIGINT_INFO bigint_complement(BIGINT *const a, const size_t size) {
	BIGINT_INFO info = 0;
	int isnegative = 0;
	/* TODO for better performance we can convert properly aligned
	 * arrays to an array of 64 bit integers (or smaller depending on
	 * the system) so we can invert multiple bytes at once */

	/* We find the two's complement of a number
	 * by inverting all bits and adding one */

	/* If the sign bit is set before computing the two's complement of 'a' and the
	 * sign bit is set after computing the two's complement of 'a' then 'a' is the
	 * most negative number, so we set a flag. */
	if(BIGINT_ISNEGATIVE(a, size)) {
		isnegative = 1;
	}

	/* Invert all bits */
	for(size_t i = 0; i < size; i++) {
		a[i] = ~(a[i]);
	}

	/* Add one */
	info |= bigint_addi(a, 1, size);

	if(BIGINT_ISNEGATIVE(a, size) && isnegative) {
		info |= BIGINT_MOSTNEG;
	}

	return info;
}

BIGINT_INFO bigint_add(BIGINT *const a, const BIGINT *const b, const size_t size) {
	BIGINT_INFO info = 0;
	const size_t digits = bigint_digits(b, size);
	int accum = 0;

	for(size_t p = 0; p < digits; p++) {
		const int sum = a[p] + b[p] + accum;

		a[p] = sum & BIGINT_DIGIT_MASK;

		accum = sum >> BIGINT_DIGIT_WIDTH;
	}

	if(accum > 0) {
		info |= bigint_carry(a, accum, digits, size);
	}

	return info;
}


BIGINT_INFO bigint_addi(BIGINT *const a, const unsigned n, const size_t size) {
	BIGINT_INFO info = 0;
	size_t p = 0;
	unsigned accum = n;

	while(accum > 0) {
		if(p >= size) {
			info |= BIGINT_OVF;
			break;
		}

		unsigned sum = (accum & BIGINT_DIGIT_MASK) + a[p];
		a[p++] = sum & BIGINT_DIGIT_MASK;

		sum >>= BIGINT_DIGIT_WIDTH;
		accum >>= BIGINT_DIGIT_WIDTH;
		accum += sum;
	}

	return info;
}

/* Subtract b from a */
/* TODO consider using a different subtraction algorithm */
BIGINT_INFO bigint_sub(BIGINT *const a, const BIGINT *const b, const size_t size) {
	BIGINT_INFO info = 0;

	if(a == b) {
		memset(a, 0, size);
	}

	BIGINT *compl = malloc(size);

	memcpy(compl, b, size);

	/* Compute two's complement of b */
	info |= bigint_complement(compl, size);

	/* Perform normal addition */
	info |= bigint_add(a, compl, size);

	free(compl);

	return info;
}

/* BIGINT multiplication: multiply /a/ by /b/, and store
 * the result in /a/.
 *
 * This method for multiplication is known as 'Long multiplication'.
 * It has O(n^2) time complexity.
 *
 * *Theoretically* multiplication can be done in O(log(n)) time
 * complexity, but in practice multiplying this way is only quicker for numbers
 * that have an ungodly amount of digits. An amount
 * we will never ever have to work with.
 *
 * Long multiplication is quicker for numbers with a small number of digits.
 * Karatsuba's method is quicker for numbers with around 1000 digits.
 *
 * TODO: Surely we can do better than O(n^2), right? Or at least
 * reduce n? TODO speed up multiplication... Somehow. */

/* XXX 0xff + 0xff == ~(0xff * 0xff) AKA ~(0xff + 0xff) == 0xff * 0xff
 * Uhh, holy shit? Is this how we multiply FAST???
 * Imagine if we could multiply by simply inverting any addition lmao.
 *
 * Does this work for any other numbers?
 * The starting point of this is the realisation that
 * 0xff + 0xff * 0xff + 0xff = 0xffff
 * This works for any base (?)
 * e.g 	99 + 99 * 99 + 99 		= 9999
 * 	0b11 + 0b11 * 0b11 + 0b11 	= 0b1111
 */

BIGINT_INFO bigint_mul(BIGINT *const a, BIGINT *b, const size_t size) {
	BIGINT_INFO info = 0;
	int cloned = 0;

	if(a == b) {
		/* If 'b' points to the same memory as 'a' then we can't do the operation 'in-place'
		 * (at least as far as I know) so we make a copy. TODO what if we let cases like this
		 * be handled by some sort of specialised square function? */
		b = malloc(size);
		memcpy(b, a, size);
		cloned = 1;
	}

	const size_t digits[] = {bigint_digits(a, size), bigint_digits(b, size)};

	/* Multiply (x * BIGINT_BASE^p) by (y * BIGINT_BASE^q) ->
	 * (x * y) * (BIGINT_BASE^(p+q))
	 *
	 * first add p and q:
	 * 	pow = p + q
	 * multiply x by y:
	 * 	prod = x * y
	 *
	 * carry the product to position pow */

	for(size_t p = digits[0]; p > 0; p--) {
		for(size_t q = digits[1]; q > 0; q--) {
			int x, y;

			x = a[p - 1];
			y = b[q - 1];

			int prod = x * y;

			if(q == 1) {
				a[p - 1] = 0;
			}

			/* FIXME we perform n^2 carries, surely we can do better */
			info |= bigint_carry(a, prod, p + q - 2, size);
		}
	}

	if(cloned) {
		free(b);
	}

	return info;
}

BIGINT_INFO bigint_muli(BIGINT *const a, const int b, const size_t size) {
	BIGINT_INFO info = 0;
	const size_t digits = bigint_digits(a, size);

	for(size_t p = digits; p > 0; p--) {
		for(size_t q = sizeof(b); q > 0; q--) {
			int x, y;

			x = a[p - 1];
			y = (b >> ((q - 1) * BIGINT_DIGIT_WIDTH)) & BIGINT_DIGIT_MASK;

			int prod = x * y;

			if(q == 1) {
				a[p - 1] = 0;
			}

			/* FIXME see bigint_mul about n^2 carries */
			info |= bigint_carry(a, prod, p + q - 2, size);
		}
	}

	return info;
}

/* Divide 'a' by 'b' and store the result in 'a', store the remainder (if not NULL) in 'rem'
 * 'a' is the dividend, 'b' is the divisor */
BIGINT_INFO bigint_div(BIGINT *const a, BIGINT *const b, BIGINT *const rem, const size_t size) {
	BIGINT_INFO info = 0;
	const size_t dvnd_digits = bigint_digits(a, size);
	const size_t dvsr_digits = bigint_digits(b, size);

	if(dvnd_digits == 0) {
		return info;
	}

	if(dvsr_digits == 0) {
		info |= BIGINT_DIV;
		return info;
	}

	if(dvnd_digits < dvsr_digits) {
		if(rem) {
			memcpy(rem, a, size);
		}
		memset(a, 0, size);
		return info;
	}

	if(a == b) {
		memset(a, 0, size);
		a[0] = 1;

		if(rem) {
			memset(rem, 0, size);
		}
		return info;
	}

	BIGINT *r = malloc(size); 	/* Remainder storage space */
	BIGINT *idvnd = malloc(size); 	/* Intermediate dividend */
	BIGINT *dvnd = malloc(size);
	uintmax_t trunc_dvsr = 0; /* The [sizeof(trunc_dvsr)] most significant digits of the divisor ('b') */

	memset(r, 0, size);
	memset(idvnd, 0, size);
	memcpy(dvnd, a, size);
	memset(a, 0, size);


	/* fill trunc_dvsr with as many significant digits as will fit */
	if(dvsr_digits > sizeof(trunc_dvsr)) {
		memcpy(&trunc_dvsr, b + dvsr_digits - sizeof(trunc_dvsr), sizeof(trunc_dvsr));
	} else {
		memcpy(&trunc_dvsr, b, dvsr_digits);
	}

	/* Set r to the first [dvsr_digits - 1] digits of the dividend */
	for(size_t i = dvsr_digits - 1, j = dvnd_digits; i > 0; i--, j--) {
		r[i - 1] = dvnd[j - 1];
	}

	for(size_t p = dvnd_digits; p >= dvsr_digits; p--) {
		const size_t i = p - 1;
		uintmax_t trunc_idvnd = 0;

		/* Multiply the remainder by 256 and add  */
		memcpy(idvnd + 1, r, size - 1);
		idvnd[0] = dvnd[i - dvsr_digits + 1];

		const size_t idvnd_digits = bigint_digits(idvnd, size);

		if(idvnd_digits > sizeof(trunc_idvnd)) {
			memcpy(&trunc_idvnd, idvnd + idvnd_digits - sizeof(trunc_idvnd), sizeof(trunc_idvnd));
		} else {
			memcpy(&trunc_idvnd, idvnd, idvnd_digits);
		}

		/* REMAINDER = INTERMEDIATE DIVIDEND - DIVISOR * beta
		 *
		 * There exists only one beta so that 0 <= REMAINDER < DIVISOR
		 *
		 * INTERMEDIATE DIVIDEND - DIVISOR * beta = 0 <= REMAINDER < DIVISOR
		 *
		 * (conjecture)
		 *
		 * We have to get the remainder as close to zero as possible
		 *
		 * Let REMAINDER = 0
		 *
		 * INTERMEDIATE DIVIDEND - DIVISOR * beta = 0 	->
		 * DIVISOR * beta = INTERMEDIATE DIVIDEND 	->
		 * beta = INTERMEDIATE DIVIDEND / DIVISOR
		 *
		 * Let base = 256
		 * beta can never be larger than the base and can never
		 * be smaller than 0 -> 0 <= beta < 256
		 *
		 * Therefore we only need to have a rough estimate of the ratio
		 * between the intermediate dividend and the divisor to figure out
		 * the value of beta.
		 *
		 * THE INTERMEDIATE DIVIDEND CAN NEVER HAVE MORE THAN ONE DIGIT OVER THE DIVISOR.
		 * This is because the remainder cannot be greater than the divisor, and we prepend a digit to
		 * the intermediate dividend. If the divisor has more digits than the
		 * intermediate dividend, then beta will equal 0 (beta = 0). */


		int beta;

		if(dvsr_digits > idvnd_digits) {
			beta = 0;
		} else if(idvnd_digits > dvsr_digits && idvnd_digits > sizeof(trunc_dvsr)) {
			/* only shift if we actually truncated the number */
			beta = trunc_idvnd / (trunc_dvsr >> BIGINT_DIGIT_WIDTH);
		} else {
			beta = trunc_idvnd / trunc_dvsr;
		}

		assert(beta < BIGINT_BASE);

		memcpy(r, b, size);
		info |= bigint_muli(r, beta, size);
		info |= bigint_sub(idvnd, r, size);
		memcpy(r, idvnd, size);

		memmove(a + 1, a, size - 1);

		a[0] = (BIGINT) beta;
	}

	if(rem) {
		memcpy(rem, r, size);
	}

	free(r);
	free(idvnd);
	free(dvnd);

	return info;
}


/* Divide 'a' by 'b' and store the result in 'a' and the remainder in 'rem' */
/* TODO with a properly aligned /a/ we can use datatypes bigger than a byte. */
BIGINT_INFO bigint_divi(BIGINT *const a, const int b, int *const rem, const size_t size) {
	if(b == 0) {
		return BIGINT_DIV;
	}

	const size_t dvnd_digits = bigint_digits(a, size);
	const size_t dvsr_digits = bigint_digits((BIGINT *) &b, sizeof(b));
	BIGINT_INFO info = 0;

	/* dividend = a */
	BIGINT *dvnd = malloc(size);

	const int dvsr = b;
	int idvnd;

	/* remainder */
	int r = 0;

	memcpy(dvnd, a, size);
	memset(a, 0, size);


	/* THE INITIAL REMAINDER SHOULD BE THE FIRST DVSR_DIGITS - 1 OF THE DIVIDEND */
	for(size_t i = dvsr_digits - 1, j = dvnd_digits; i > 0; i--, j--) {
		r |= (dvnd[j - 1]) << ((i - 1) * BIGINT_DIGIT_WIDTH);
	}

	/* INTERMEDIATE DIVIDEND = BASE * REMAINDER + DIGIT OF DIVISOR AT POS p + dvnd_digits - 1 */
	for(size_t p = dvnd_digits; p >= dvsr_digits; p--) {
		const size_t i = p - 1;

		/* Add the remainder to the front of the intermediate dividend*/
		idvnd = r << BIGINT_DIGIT_WIDTH;
		idvnd |= dvnd[i - dvsr_digits + 1];

		const int beta = idvnd / dvsr;
		assert(beta < BIGINT_BASE);

		r = idvnd - (dvsr * beta);

		memmove(a + 1, a, size - 1);
		a[0] = (BIGINT) beta;
	}

	if(rem) {
		*rem = r;
	}

	free(dvnd);

	return info;
}

/* Exponentiation by squaring
 *
 * The reason this works:
 * 	https://cp-algorithms.com/algebra/binary-exp.html */
/* TODO what if /a/ is negative? */
BIGINT_INFO bigint_pow(BIGINT *const a, int p, const size_t size) {
	BIGINT_INFO info = 0;

	BIGINT *res = a;
	BIGINT *in = malloc(size);

	memcpy(in, a, size);
	memset(res, 0, size);

	/* res = 1 */
	res[0] = 1;

	while(p > 0) {
		if(p & 1) {
			/* p is odd */
			/* res = res * a */
			info |= bigint_mul(res, in, size);
		}

		/* Square */
		/* a = a * a */
		info |= bigint_mul(in, in, size);

		/* p = p / 2 */
		p >>= 1;
	}

	free(in);

	return info;
}
