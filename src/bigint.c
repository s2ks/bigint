#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#include "bigint.h"

/*#include <stdio.h>*/

/* XXX: document, document, document...
 * Seriously, its better to have too many comments
 * than too little. If someone actually has issues with
 * too many comments then they can easily remove them
 * with a regex. But missing comments can't be conjured
 * out of thin air. So comment, comment, comment. State
 * the obvious a thousand times. */

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


#if 0
BIGINT *bigint_add(BIGINT *dest, const BIGINT *a, const BIGINT *b) {
	const int digits = bigint_digits(b);

	bigint_set(dest, a);

	for(int i = 0; i < digits; i++) {
		bigint_carry(dest, i, b->buf[i]);
	}

	return dest;
}

/* FIXME what if we add a negative? */
BIGINT *bigint_addi(BIGINT *dest, const BIGINT *a, const int n) {
	BIGINT_BUFFER bbuf[BIGINT_SIZETOLEN(sizeof(n))];
	BIGINT b = BIGINT_ASSIGN(bbuf, sizeof(bbuf));

	bigint_itob(&b, n);

	bigint_add(dest, a, &b);

	return dest;
}

/* Subtract b from a */
/* TODO consider using a different subtraction algorithm */
BIGINT *bigint_sub(BIGINT *dest, const BIGINT *a, const BIGINT *b) {
	/* We set bcompl_buf equal to the size of dest because
	 * dest has to 'overflow' correctly for the proper
	 * value to be represented. */
	BIGINT_BUFFER bcompl_buf[BIGINT_CAP(dest)];
	BIGINT bcompl = BIGINT_ZERO_INIT(bcompl_buf, sizeof(bcompl_buf));

	bigint_set(&bcompl, b);

	/* Convert b to a negative if it's a positive value,
	 * otherwise convert to a positive if it's a negative
	 * value. */

	/* FIXME what if b is most negative value?
	 *
	 * we still subtract even though we should
	 * add.
	 *
	 * Maybe set a flag? */

	/* Compute two's complement of b */
	bigint_complement(&bcompl, b);

	/* Perform normal addition */
	bigint_add(dest, a, &bcompl);

	return dest;
}
#endif

/* BIGINT multiplication: multiply a by b, and store
 * the result in dest.
 *
 * This method for multiplication is known as 'Long multiplication'.
 * It has O(n^2) time complexity (assuming a->size = b->size).
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

/* FIXME what if we multiply a negative? */

/* TODO how do we utilise memory cleverly to efficiently multiply two
 * arrays of digits? */

/* TODO if a and b do not have the same capacity then the smallest one
 * should be promoted. And sign extended if it is a negative
 * (Check with BIGINT_ISNEGATIVE()*/

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

BIGINT_INFO bigint_mul(BIGINT *a, BIGINT *b, const size_t size) {
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

			info |= bigint_carry(a, prod, p + q - 2, size);
		}
	}

	if(cloned) {
		free(b);
	}

	return info;
}

/* FIXME take a look at bigint_mul and take lessons learned from implementing that,
 * also this is broken */
BIGINT_INFO bigint_muli(BIGINT *a, const int b, const size_t size) {
	BIGINT_INFO info = 0;
	const size_t digits = bigint_digits(a, size);

	for(size_t i = digits; i > 0; i--) {
		const BIGINT_DIGIT digit = a[i - 1];
		/* FIXME no need to split this into individual bytes, we can just multiply
		 * each element in 'a' with the entirety of 'b' */
		for(size_t j = sizeof(b); j > 0; j--) {
			const int prod = digit * ((b >> ((j - 1) * BIGINT_DIGIT_WIDTH)) & BIGINT_DIGIT_MASK);

			/*printf("%d\n", prod);*/
			if(prod) {
				info |= bigint_carry(a, prod, i + j - 2, size);
			}
		}
		a[i - 1] -= digit;
	}

	return info;
}

/* Division: how many times does the divisor fit in the dividend?
 *
 * rem: pointer to a location to store the remainder, optional, can be NULL
 *
 * let n be the divisor, and m be the dividend
 *
 * store the quotient in n
 *
 * Algorithm shamelessly stolen from:
 * 	https://en.wikipedia.org/wiki/Long_division#Algorithm_for_arbitrary_base */
#if 0
/* FIXME new usage. Alternatively: rewrite using long division */
BIGINT_INFO bigint_div(BIGINT *a, const BIGINT *b, BIGINT *rem, size_t size) {
	BIGINT_INFO info = 0;
	BIGINT *dividend = malloc(size);
	BIGINT *divisor = b;

	memcpy(dividend, a, size);

	memset(a, 0, size);

	/* count the number of digits in the dividend (n) and the divisor (m) */
	const size_t dvnd_digits = bigint_digits(dividend, size); /* k */
	const size_t dvsr_digits = bigint_digits(divisor, size);  /* l */

	/* let k be the number of digits in n (dividend)
	 * let l be the number of digits in m (divisor)
	 *
	 * each digit is base 256 unless BIGINT_BASE says otherwise */

	/* if k < l then the quotient is 0 and the remainder
	 * is n */
	if(dvnd_digits < dvsr_digits) {
		if(rem) {
			memcpy(rem, dividend, size);
		}

		return info;
	}

	/* We store the quotient in 'a' */
	BIGINT *q = a;
	BIGINT *r = malloc(size);

	memset(r, 0, size);

	BIGINT_BUFFER d_buf[BIGINT_CAP(dividend)];
	BIGINT_BUFFER t_buf[BIGINT_CAP(dividend)];

	/* Intermediate dividend */
	BIGINT idvnd 	= BIGINT_ZERO_INIT(d_buf, sizeof(d_buf));

	/* Temporary storage */
	BIGINT temp 	= BIGINT_ZERO_INIT(t_buf, sizeof(t_buf));

	/* Next digit of the quotient */
	uintmax_t beta;

	/* Set r to the first dvsr_digits - 1 digits of the dividend */
	for(size_t i = dvnd_digits, j = dvsr_digits - 1; j > 0; i--, j--) {
		r[j - 1] = dividend[i - 1];
	}
	/* FIXME: What if l = 0? Should l be allowed to be 0? Can l ever be zero? */
	assert(dvnd_digits > 0);
	assert(dvsr_digits > 0);
	assert(dvnd_digits >= dvsr_digits);

	/* Iterate from 0 to k-l -> dvdnd_digits - dvsr_digits */
	for(size_t counter = dvnd_digits; counter >= dvsr_digits; counter--) {
		const size_t i = counter - 1;
		assert(counter > 0);
		/* INTERMEDIATE DIVIDEND = BASE * REMAINDER + DIGIT OF DIVISOR AT POS
		 * i + dvnd_digits - 1
		 *
		 * This assumes position of digit is from left to right, with the most
		 * significant digit as position 0 and the least significant digit at position
		 * ndigits - 1.
		 * But BIGINT digits go from right to left, with the least significant
		 * digit at position 0 and the most significant digit at position
		 * ndigits - 1 so we take the digit of the divisor at position
		 * i - dvnd_digits + 1 instead */

		/*bigint_set(&idvnd, bigint_muli(&temp, &r, BIGINT_BASE));*/
		bigint_set(&idvnd, &r);
		bigint_dshl(&idvnd, 1);

		/* FIXME The following can simply be rewritten as
		 * idvnd.buf[0] = dividend->buf[i-dvsr_digits+1] */
		bigint_addi(&idvnd, &idvnd, dividend->buf[i - dvsr_digits + 1]);

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
		 * The intermediate dividend can never have more than one digit
		 * over the divisor. If the divisor has more digits than the
		 * intermediate dividend, then beta will equal 0 (beta = 0).
		 *
		 * Therefore we only need to have a rough estimate of the ratio
		 * between the intermediate dividend and the divisor to figure out
		 * the value of beta. */

		/* In order to figure out beta we need to represent
		 * a ratio between the intermediate dividend and the divisor
		 *
		 * We do this by trying to fit as many (most significant) digits of both
		 * the divisor and the intermediate dividend (idvnd),
		 * in trunc_dvsr and trunc_idvnd respectively, as possible. */
		/* FIXME truncated divisor is constant */
		uintmax_t trunc_dvsr 		= 0; /* m */
		uintmax_t trunc_idvnd 		= 0; /* d */
		const size_t idvnd_digits 	= bigint_digits(&idvnd);

		/* Number of iterations needed to fill trunc_dvsr and trunc_idvnd
		 * respectively. AKA maximum number of digits that can fit
		 * in trunc_dvsr and trunc_idvnd */
		const size_t dvsr_iter = MAX(sizeof(trunc_dvsr) / sizeof(BIGINT_BUFFER), 1);
		const size_t idvnd_iter = MAX(sizeof(trunc_idvnd) / sizeof(BIGINT_BUFFER), 1);

		const size_t dvsr_stop = dvsr_digits > dvsr_iter ?
			dvsr_digits - dvsr_iter : 0;
		const size_t idvnd_stop = idvnd_digits > idvnd_iter ?
			idvnd_digits - idvnd_iter : 0;

		/* FIXME truncated divisor is constant */
		/* Fill trunc_dvsr with as many significant digits as possible */
		for(size_t x = dvsr_digits; x > dvsr_stop; x--) {
			trunc_dvsr = trunc_dvsr << (sizeof(BIGINT_BUFFER) * CHAR_BIT);
			trunc_dvsr |= divisor->buf[x - 1];
		}

		/* Fill trunc_idvnd with as many significant digits as possible */
		for(size_t x = idvnd_digits; x > idvnd_stop; x--) {
			trunc_idvnd = trunc_idvnd << (sizeof(BIGINT_BUFFER) * CHAR_BIT);
			trunc_idvnd |= idvnd.buf[x - 1];
		}

		/* How many digits did NOT fit in trunc_idvnd? */
		const size_t ntrunc_idvnd = idvnd_iter > idvnd_digits ? 0 :
			idvnd_digits - idvnd_iter;

		/* How many digits did NOT fit in trunc_dvsr? */
		const size_t ntrunc_dvsr = dvsr_iter > dvsr_digits ? 0 :
			dvsr_digits - dvsr_iter;

		/* Correct trunc_dvsr and trunc_idvnd for the proper ratio
		 *
		 * If the divisor is smaller than the intermediate dividend, then
		 * we need to correct trunc_dvsr so the right ratio is represented.
		 *
		 * The same applies for if the intermediate dividend is smaller than the
		 * divisor, but in that case we can take a shortcut and just set
		 * beta to zero. */

		if(ntrunc_idvnd > ntrunc_dvsr) {
			trunc_dvsr = trunc_dvsr >> (ntrunc_idvnd - ntrunc_dvsr) *
				(sizeof(BIGINT_BUFFER) * CHAR_BIT);

			/* Help debugging division by zero. The
			 * "Floating point exception" that is raised
			 * otherwise is not very helpful. */
			ABORT(trunc_dvsr == 0);

			beta = trunc_idvnd / trunc_dvsr;
		} else if(ntrunc_dvsr > ntrunc_idvnd) {
			/* no need to do division, beta is zero */
			beta = 0;
		} else {
			/* Number of digits in idvnd and dvsr
			 * are equal */
			beta = trunc_idvnd / trunc_dvsr;
		}

		/* beta *should* never be able to be
		 * larger than BIGINT_BASE - 1
		 * If it is, then the implementation of
		 * this algorithm is likely flawed. */
		assert(beta < BIGINT_BASE);

		/* calculate the remainder
		 * FIXME This set can be removed */
		bigint_set(&r, &idvnd);

		/* FIXME subtract from idvnd, set the result in r */
		bigint_sub(&r, &r, bigint_muli(&temp, divisor, beta));

		bigint_dshl(&q, 1);
		q.buf[0] = (BIGINT_BUFFER) beta;
	}

	/* Store the quotient */
	bigint_set(dest, &q);

	/* Store the remainder in rem */
	if(rem) {
		bigint_set(rem, &r);
	}

	free(dividend);
	free(r);

	return dest;
}
#endif


/* Divide 'a' by 'b' and store the result in 'a' and the remainder in 'rem'
 * TODO do we want the remainder? [Update] YES */
BIGINT_INFO bigint_divi(BIGINT *a, const int b, int *rem, const size_t size) {
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

		/*memset(idvnd, 0, size);*/

		/* Add the remainder to the front of the intermediate dividend*/
		idvnd = r << BIGINT_DIGIT_WIDTH;
		idvnd |= dvnd[i];

		const int beta = idvnd / dvsr;
		assert(beta < BIGINT_BASE);

		r = idvnd - (dvsr * beta);

		bigint_dshl(a, 1, size);
		a[0] = (BIGINT) beta;
	}


	/* TODO while the remainder is larger or equal to b carry a 1 to position
	 * 0, aka divide by b and carry the quotient. */

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
/* TODO test me */
BIGINT_INFO bigint_pow(BIGINT *a, int p, const size_t size) {
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
