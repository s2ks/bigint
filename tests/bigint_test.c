#include <stdio.h>
#include <string.h>
#include <assert.h>

#define __DEBUG_OUT stdout

#include "bigint.h"

enum {
	TEST_SET,
	TEST_COMPL,
	TEST_ADD,
	TEST_SUB,
	TEST_MUL,
	TEST_DIV,
	TEST_POW,

	/* ---- */
	TEST_COUNT,
};

/* There is nothing special about these values, they are arbitrarily chosen.
 * I realise that this is probably a bad way of testing correctness. FIXME? */
static const BIGINT *testres[] = {
	(BIGINT[64]) {0x00, 0x91, 0xc3, 0x47, 0x2d, 0x36}, 	/*59568105427200*/
	(BIGINT[64]) {0x64}, 					/*100*/
	(BIGINT[64]) {0x00, 0xe1, 0xf5, 0x05}, 			/*100000000*/
};

void test_mul(void);
void test_set(void);
void test_div(void);
void test_add(void);
void test_sub(void);
void test_pow(void);
void test_compl(void);

static void (*const test[TEST_COUNT])(void) = {
	[TEST_SET] = test_set,
	[TEST_COMPL] = test_compl,
	[TEST_ADD] = test_add,
	[TEST_SUB] = test_sub,
	[TEST_MUL] = test_mul,
	[TEST_DIV] = test_div,
	[TEST_POW] = test_pow,
};

void test_set(void) {
	BIGINT a[64];
	BIGINT r[64];

	memcpy(r, (BIGINT[64]) {0x00, 0xd5, 0x02, 0xa4, 0x56, 0x95, 0x08, 0xc7, 0x3e, 0x00, 0x5c, 0x00, 0x00, 0x00, 0x00, 0x00}, sizeof(r));

	bigint_set(a, "111222333444555666777888000", sizeof(a));

	assert(memcmp(a, r, sizeof(a)) == 0);

	bigint_set(a, "12345e120", sizeof(a));

	memcpy(r, (BIGINT[64]) {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x59, 0xe3, 0x02, 0xa4,
		0x72, 0xfc, 0x86, 0xd5, 0x37, 0xf5, 0x4c, 0xe2, 0xd3, 0x34, 0xcf, 0x44, 0xf7, 0x72, 0x6a, 0x0b, 0x03, 0x80, 0xd4,
		0x95, 0x68, 0xa5, 0x8c, 0x8a, 0x50, 0x10, 0xce, 0x0d, 0xd1, 0xf7, 0xb6, 0xac, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	}, sizeof(r));

	assert(memcmp(a, r, sizeof(a)) == 0);

	/* TODO more tests once we support setting negative values, also test invalid value strings */
	/* TODO test setting a value greater than what would fit in a given size */

	printf("test_set: All tests passed.\n");
}

void test_div(void) {
	BIGINT a[64] = {0xff, 0xef, 0xee, 0xdd, 0xcd, 0xcc, 0xbb, 0xab, 0xaa, 0x99, 0x89, 0x88, 0x77, 0x67, 0x66, 0x55,
			0x45, 0x44, 0x33, 0x23, 0x22, 0x11, 0x01};
	BIGINT b[64] = {0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	BIGINT q[64] = {0x06, 0x26, 0xfd, 0x54, 0xe7, 0xc8, 0x7e, 0x5b, 0x93, 0xec, 0x36, 0x4a, 0x5a, 0x12, 0x01, 0x00};
	BIGINT r[64] = {0x9f, 0x63, 0xb7, 0xde, 0x97, 0x8d, 0x66, 0x9e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	BIGINT rem[64];

	bigint_print(a, sizeof(a));
	bigint_print(b, sizeof(b));

	bigint_div(a, b, rem, sizeof(a));

	bigint_print(a, sizeof(a));
	bigint_print(rem, sizeof(rem));

	assert(memcmp(a, q, sizeof(a)) == 0);
	assert(memcmp(rem, r, sizeof(r)) == 0);

	printf("test_div: All tests passed.\n");
}

void test_add(void) {}
void test_sub(void) {
	BIGINT a[64] = {0xbb, 0xbb, 0xbb, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50};
	BIGINT b[64] = {0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xee, 0xee, 0xff, 0x40};
	BIGINT r[64] = {0x11, 0x11, 0x11, 0x00, 0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x12, 0x11, 0x00, 0x0f};

	bigint_sub(a, b, sizeof(a));

	assert(memcmp(a, r, sizeof(r)) == 0);

	/* TODO support setting negative values */
	/*bigint_set(a, "5e100", sizeof(a));*/
	/*bigint_set(b, "5e101", sizeof(b));*/
	/*bigint_set(r, "-45e100", sizeof(r));*/

	/*bigint_sub(a, b, sizeof(a));*/

	/*assert(memcmp(a, r, sizeof(a)) == 0);*/

	printf("test_sub: All tests passed.\n");
}
void test_pow(void) {}
void test_compl(void) {}

void test_mul(void) {
	BIGINT a[64] = {0x10, 0xee, 0xab};
	BIGINT b[64] = {0x10, 0xab, 0x50};
	BIGINT r[64];

	bigint_mul(a, b, sizeof(a));

	assert(memcmp(a, testres[0], sizeof(a)) == 0);

	memset(a, 0, sizeof(a));
	a[0] = 10;

	bigint_mul(a, a, sizeof(a));

	assert(memcmp(a, testres[1], sizeof(a)) == 0);

	memset(a, 0, sizeof(a));
	a[0] = 0x10;
	a[1] = 0x27;

	bigint_mul(a, a, sizeof(a));

	assert(memcmp(a, testres[2], sizeof(a)) == 0);

	printf("test_mul: All tests passed.\n");
}

int main(void) {
	BIGINT a[64];
	bigint_set(a, "1e150", sizeof(a));

	bigint_print(a, sizeof(a));

	bigint_set(a, "1000", sizeof(a));

	bigint_print(a, sizeof(a));

	bigint_divi(a, 10, NULL, sizeof(a));

	bigint_print(a, sizeof(a));

	bigint_set(a, "12298347289347298374983479333333333333472938888888883333333333333333333333333333333333333333333333333", sizeof(a));
	bigint_print(a, sizeof(a));

	bigint_complement(a, sizeof(a));
	bigint_print(a, sizeof(a));

	for(int i = 0; i < TEST_COUNT; i++) {
		test[i]();
	}

	return 0;
}
