#include <stdio.h>
#include <string.h>
#include <assert.h>

#define __DEBUG_OUT stdout

#include "bigint.h"

static const BIGINT *testres[] = {
	(BIGINT[64]) {0x00, 0x91, 0xc3, 0x47, 0x2d, 0x36},
	(BIGINT[64]) {0x64},
	(BIGINT[64]) {0x00, 0xe1, 0xf5, 0x05},
};

void test_mul(void);
void test_set(void);

static const void (*test[])(void) = {
	test_mul,
	test_set,
};

void test_set(void) {

}


void test_mul(void) {
	BIGINT a[64] = {0x10, 0xee, 0xab};
	BIGINT b[64] = {0x10, 0xab, 0x50};

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

	_bigint_dbgprint(a, sizeof(a));

	assert(memcmp(a, testres[2], sizeof(a)) == 0);
}

int main(void) {

	/* TODO this should be the usage */
	BIGINT a[64];
	bigint_set(a, "1e150", sizeof(a));

	/*bigint_print(a, sizeof(a));*/
	_bigint_dbgprint(a, sizeof(a));

	bigint_set(a, "1000", sizeof(a));

	bigint_divi(a, 10, NULL, sizeof(a));

	_bigint_dbgprint(a, sizeof(a));


	/*test_mul();*/

	/*BIGINT a[64] = {10};*/

	/*bigint_pow(a, 20, sizeof(a));*/

	/*_bigint_dbgprint(a, sizeof(a));*/

	return 0;
}
