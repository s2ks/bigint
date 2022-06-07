#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#define __DEBUG_OUT stdout

#include "bigint.h"
#include "util.h"

/* XXX: document, document, document...
 * Seriously, its better to have too many comments
 * than too little. If someone actually has issues with
 * too many comments then they can easily remove them
 * with a regex. But missing comments can't be conjured
 * out of thin air. So comment, comment, comment. State
 * the obvious a thousand times. */

/* TODO int to bigint and bigint to int conversion helpers */

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
void _bigint_dbgprint(BIGINT *a, size_t size) {
	const int cap = size;

	for(int i = 0; i < cap; i++) {
		fprintf(__DEBUG_OUT, "%02x ", a[i]);
	}
	fprintf(__DEBUG_OUT, "\n");
}

static inline size_t bigint_digits(const BIGINT *big) {
	if(big == NULL) {
		return 0;
	}

	const size_t cap = BIGINT_CAP(big);

	for(size_t i = cap; i > 0; i--) {
		if(big->buf[i - 1] != 0) {
			return i;
		}
	}

	return 0;
}

/* TODO return type? */
static inline void _bigint_buf_seti(BIGINT_BUFFER *dest, size_t size, uintmax_t val) {
	const size_t 	cap = size / sizeof(*dest);
	uintmax_t 	accum;

	do {


	} while(val > 0);
}
#endif


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

BIGINT *bigint_mul(BIGINT *dest, const BIGINT *a, const BIGINT *b) {
	const size_t dest_cap = BIGINT_CAP(dest);

	/* We create a temporary storage space for the result because
	 * we could have dest == a || dest == b */
	BIGINT_BUFFER tmp_buf[dest_cap];
	BIGINT_BUFFER *org_buf = dest->buf;

	memset(tmp_buf, 0, sizeof(tmp_buf));

	dest->buf = tmp_buf;

	/* Multiply (x * BIGINT_BASE^p) by (y * BIGINT_BASE^q) ->
	 * (x * y) * (BIGINT_BASE^(p+q))
	 *
	 * first add p and q:
	 * 	pow = p + q
	 * multiply x by y:
	 * 	prod = x * y
	 *
	 * carry the product to position pow */

	for(size_t p = 0; p < BIGINT_CAP(a); p++) {
		for(size_t q = 0; q < BIGINT_CAP(b); q++) {
			BIGINT_WORD x, y;

			x =  a->buf[p];
			y =  b->buf[q];

			bigint_carry(dest, q + p, x * y);
		}
	}
	memcpy(org_buf, tmp_buf, sizeof(tmp_buf));

	dest->buf = org_buf;

	return dest;
}

/* FIXME what if we multiply by a negative value? */
BIGINT *bigint_muli(BIGINT *dest, const BIGINT *a, const int n) {
	const size_t dest_cap = BIGINT_CAP(dest);
	const size_t digits = bigint_digits(a);

	BIGINT_BUFFER *org_buf = dest->buf;
	BIGINT_BUFFER dtmp_buf[dest_cap];
		

	if(n < 0) {
		_bigint_buf_seti(nbuf, sizeof(nbuf), -n);
	} else {
		_bigint_buf_seti(nbuf, sizeof(nbuf), n);
	}



	/* Consider what happens if 'dest' == 'a',
	 * we can't clear 'dest' because we might be
	 * clearing 'a'. So we have to allocate
	 * a temporary storage space. */
	BIGINT_BUFFER store_buf[BIGINT_CAP(dest)];
	BIGINT store = BIGINT_ZERO_INIT(store_buf, dest->size);

	for(size_t i = 0; i < digits; i++) {
		uintmax_t prod;

		prod = a->buf[i] * n;

		bigint_carry(&store, i, prod);
	}

	/* Copy the contents of 'store' to 'dest' */
	bigint_set(dest, &store);

	return dest;
}

/* Booth's multiplication algorithm (attempt)
 * https://en.wikipedia.org/wiki/Booth%27s_multiplication_algorithm */

/*TODO*/

#if 0
static BIGINT *bigint_booth_mul(BIGINT *dest, const BIGINT *m, const BIGINT *r) {
	const size_t x = BIGINT_CAP(m);
	const size_t y = BIGINT_CAP(r);
	const size_t len = x + y + 1;

	BIGINT_BUFFER buf1[len];
	BIGINT_BUFFER buf2[len];
	BIGINT_BUFFER buf3[len];

	BIGINT A = BIGINT_ASSIGN(buf1, sizeof(buf1));
	BIGINT S = BIGINT_ASSIGN(buf2, sizeof(buf2));
	BIGINT P = BIGINT_ASSIGN(buf3, sizeof(buf3));


	for(size_t i = 0; i < len; i++) {
		/* Fill the x most significant bits with m, the rest with 0 */
		A.buf[i] = i >= len - x ? m->buf[i - (len - x)] : 0;
		S.buf[i] = i >= len - x ? ~(m->buf[i - (len - x)]) : 0;
		P.buf[i] = i < BIGINT_CAP(r) ? r->buf[i] : 0;
	}

	bigint_carry(S, len - x, 1);
}
#endif

/* Karatsuba algorithm (attempt)
 *
 * reference: https://en.wikipedia.org/wiki/Karatsuba_algorithm */

/* FIXME with really large numbers we might get a stack overflow, unfortunately
 * we *have* to use recursion because we essentially have to walk a tree.
 *
 * An alternative would be to use a stack data structure and 'recurse' that
 * way in a loop. This data structure would be allocated on the heap so
 * we need to determine the tree depth before allocating.
 *
 * As it stands, this implementation becomes completely unusable
 * for sufficiently large numbers long before the stack even gets
 * close to overflowing. (probably?) */

/* TODO REMOVE ME */
#if 0
static BIGINT *bigint_mul_recurse(BIGINT *dest, const BIGINT *a,
		const BIGINT *b, const size_t alen, const size_t blen)
{
	if(alen == 0 || blen == 0) {
		bigint_set(dest, bigint_const[ZERO_VAL]);
		return dest;
	}

	assert(alen > 0);
	assert(blen > 0);

	if(alen == 1 || blen == 1) {
		return bigint_long_mul(dest, a, b);
	}

	const size_t n = MIN(alen, blen) / 2;
	const size_t l = MAX(alen, blen) - n;

	assert(n > 0);

	const BIGINT a_hi = BIGINT_ASSIGN_UNSIGNED(&(a->buf[n]), BIGINT_LENTOSIZE(alen - n));
	const BIGINT a_lo = BIGINT_ASSIGN_UNSIGNED(&(a->buf[0]), BIGINT_LENTOSIZE(n));
	const BIGINT b_hi = BIGINT_ASSIGN_UNSIGNED(&(b->buf[n]), BIGINT_LENTOSIZE(blen - n));
	const BIGINT b_lo = BIGINT_ASSIGN_UNSIGNED(&(b->buf[0]), BIGINT_LENTOSIZE(n));

	assert(l >= n);
	BIGINT_BUFFER t1buf[l + l];
	BIGINT_BUFFER t2buf[l + l];
	BIGINT_BUFFER combprod_buf[(l + l) * 2];

	BIGINT t1 = BIGINT_ASSIGN_UNSIGNED(t1buf, sizeof(t1buf));
	BIGINT t2 = BIGINT_ASSIGN_UNSIGNED(t2buf, sizeof(t2buf));
	BIGINT combprod = BIGINT_ASSIGN_UNSIGNED(combprod_buf, sizeof(combprod_buf));

	/* lo is in range of [0..n]
	 * hi is in range of [n..digits] */

	bigint_add(&t1, &a_hi, &a_lo); /* is at most (alen - n + 1) digits long */
	bigint_add(&t2, &b_hi, &b_lo); /* is at most (blen - n + 1) digits long */

	/* asum := a_hi + a_lo
	 * bsum := b_hi + b_lo
	 * combprod := asum * bsum */

	/* is at most (alen - n + 1) + (blen - n + 1) digits long */
	bigint_mul_recurse(&combprod, &t1, &t2,
			bigint_digits(&t1), bigint_digits(&t2));

	/* FIXME t1 (loprod) most significant digit is off by one */

	/* is at most n + n digits long */
	bigint_mul_recurse(&t1, &a_lo, &b_lo, n, n);

	/* is at most (alen - n) + (blen - n) digits long */
	bigint_mul_recurse(&t2, &a_hi, &b_hi, alen - n, blen - n);

	/* loprod := a_lo * b_lo */
	/* hiprod := a_hi * b_hi */

	bigint_set(dest, &t2);
	bigint_dshl(dest, n);

	bigint_sub(&combprod, &combprod, &t2);
	bigint_sub(&combprod, &combprod, &t1);

	/* midprod := combprod - hiprod - loprod =  */

	bigint_add(dest, dest, &combprod);
	bigint_dshl(dest, n);

	bigint_add(dest, dest, &t1);

	/* a*b = (hiprod * base^(n*2)) + (midprod * base^n) + loprod
	 * We perform this operation in the order of:
	 * ((((hiprod) * base^n) + midprod) * base^n) + loprod */

	return dest;
}

BIGINT *bigint_mul(BIGINT *dest, const BIGINT *a, const BIGINT *b) {
	const size_t digits_a = bigint_digits(a);
	const size_t digits_b = bigint_digits(b);

	/* FIXME if either a or b is zero (has zero digits) the
	 * result is zero. */
	assert(digits_a > 0);
	assert(digits_b > 0);

	bigint_mul_recurse(dest, a, b, digits_a, digits_b);

	return dest;
}
#endif

/* TODO implement
 *
 * CONCLUSION:
 * 	I don't think it's possible to implement this
 * 	iteratively without using some kind of stack structure.
 *
 * 	Essentially we have to walk a tree, for which we need
 * 	to use recursion.
 *
 * 	The recursive implementation of this function
 * 	can be optimised using tail-call recursion, so we can
 * 	allocate all our memory once, and then walk the tree using
 * 	recursion.
 *
 * 	I don't see a reason for using a stack data structure,
 * 	I doubt it would be any faster than using recursion.
 * 	Unless we start getting stack overflows, using recursion is
 * 	probably fine.
 *
 * 	Okay so tail-call recursion is basically a different way of
 * 	writing a loop. Which means this can't actually be
 * 	optimised using tail-call recursion.
 *
 * 	TODO: add a stack data structure to my util library?
 * 	(oh! And what about a self-balancing binary search tree?)
 * */

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

BIGINT *bigint_div(BIGINT *dest, const BIGINT *dividend, const BIGINT *divisor,
		BIGINT *rem)
{
	/* count the number of digits in the dividend (n) and the divisor (m) */
	const size_t dvnd_digits = bigint_digits(dividend); /* k */
	const size_t dvsr_digits = bigint_digits(divisor);  /* l */

	/* let k be the number of digits in n (dividend)
	 * let l be the number of digits in m (divisor)
	 *
	 * each digit is base 256 unless BIGINT_BASE says otherwise */

	/* if k < l then the quotient is 0 and the remainder
	 * is n */
	if(dvnd_digits < dvsr_digits) {
		if(rem) {
			bigint_set(rem, dividend);
		}

		bigint_seti(dest, 0);
		return dest;
	}

	BIGINT_BUFFER q_buf[BIGINT_CAP(dividend)];
	BIGINT_BUFFER r_buf[BIGINT_CAP(dividend)];
	BIGINT_BUFFER d_buf[BIGINT_CAP(dividend)];
	BIGINT_BUFFER t_buf[BIGINT_CAP(dividend)];

	/* Quotient */
	BIGINT q 	= BIGINT_ZERO_INIT(q_buf, sizeof(q_buf));

	/* Remainder */
	BIGINT r 	= BIGINT_ZERO_INIT(r_buf, sizeof(r_buf));

	/* Intermediate dividend */
	BIGINT idvnd 	= BIGINT_ZERO_INIT(d_buf, sizeof(d_buf));

	/* Temporary storage */
	BIGINT temp 	= BIGINT_ZERO_INIT(t_buf, sizeof(t_buf));

	/* Next digit of the quotient */
	uintmax_t beta;

	/* Set r to the first dvsr_digits - 1 digits of the dividend */
	for(size_t i = dvnd_digits, j = dvsr_digits - 1; j > 0; i--, j--) {
		r.buf[j - 1] = dividend->buf[i - 1];
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

	return dest;
}

BIGINT *bigint_divi(BIGINT *dest, const BIGINT *dividend, const int divisor, int *rem) {
	const size_t buflen = MAX(sizeof(divisor) / sizeof(BIGINT_BUFFER), 1);

	BIGINT_BUFFER dvsr_buf[buflen];
	BIGINT_BUFFER rem_buf[buflen];

	BIGINT bdvsr 	= BIGINT_INIT(dvsr_buf, sizeof(dvsr_buf), divisor);
	BIGINT brem 	= BIGINT_ZERO_INIT(rem_buf, sizeof(rem_buf));

	bigint_div(dest, dividend, &bdvsr, &brem);

	if(rem == NULL) {
		return dest;
	}

	*rem = 0;
	for(size_t i = buflen; i > 0; i--) {
		*rem <<= BIGINT_DIGIT_WIDTH;
		*rem |= brem.buf[i - 1];
	}

	return dest;
}

/* Exponentiation by squaring
 *
 * The reason this works:
 * 	https://cp-algorithms.com/algebra/binary-exp.html */
/* TODO test me */
BIGINT *bigint_pow(BIGINT *dest, const BIGINT *a, intmax_t p) {
	if(p < 0) {
		bigint_seti(dest, 0);
		return dest;
	} else if(p == 0) {
		bigint_seti(dest, 1);
		return dest;
	} else if(p == 1) {
		bigint_set(dest, a);
		return dest;
	}

	BIGINT_BUFFER tbuf[BIGINT_CAP(a)];
	BIGINT temp = BIGINT_ZERO_INIT(tbuf, sizeof(tbuf));

	bigint_seti(dest, 1);
	bigint_set(&temp, a);

	while(p > 0) {
		if(p & 1) {
			/* p is odd aka lsb is set so we
			 * need to set dest to the product
			 * of dest and the squared result of the
			 * previous iteration */
			bigint_mul(dest, dest, &temp);
		}

		/* Square */
		bigint_mul(&temp, &temp, &temp);

		/* p = p / 2 */
		p >>= 1;
	}

	return dest;
}

#if 0

/* TODO: support hexadecimal */
int _bigint_stob(BIGINT_BUFFER *buf, size_t size, const char *str) {
	if(str) {
		return _bigint_stob_base10(buf, size, str);
	} else {
		return 0;
	}
}

#endif

#if 0
/* Convert a string representing a base 10 number to BIGINT format */
BIGINT *bigint_strtobig_base10(BIGINT *dest, const char *str) {
	const size_t slen = strlen(str);

	BIGINT_BUFFER *pbuf = malloc(dest->size); ABORT(pbuf == NULL);
	BIGINT p = {.buf = pbuf, .size = dest->size};

	for(size_t i = slen, q = 0; i > 0; i--, q++) {
		const int digit = str[i - 1] - '0';

		if(digit > 9 || digit < 0) {
			dest->flag |= BIGINT_NAN;
			return dest;
		}

		memset(p.buf, 0, p.size);

		bigint_exp10(&p, q); /* TODO implement exp10 */
		bigint_muli(&p, &p, digit);

		bigint_add(dest, dest, &p);
	}

	return dest;
}
#endif
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
