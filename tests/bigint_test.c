#include <stdio.h>
#include <sys/random.h>

#include <gperftools/profiler.h>

#define __DEBUG_OUT stdout

#include "bigint.h"
#include "util.h"

void bigint_dump(const BIGINT *a) {
	size_t cap = BIGINT_CAP(a);

	for(size_t i = 0; i < cap; i++) {
		printf("%x ", a->buf[i]);
	}

	printf("\n");
}

void test0(void) {
	BIGINT_BUFFER 	buf1[16];
	BIGINT_BUFFER 	buf2[16];
	BIGINT_BUFFER 	buf3[16];
	BIGINT 		b1 = BIGINT_INIT(buf1, sizeof(buf1), 123456);
	BIGINT 		b2 = BIGINT_INIT(buf2, sizeof(buf2), 333);
	BIGINT 		b3 = BIGINT_ASSIGN(buf3, sizeof(buf3));
	char string[16];

	memset(string, 'A', sizeof(string));

	bigint_tostr(&b1, sizeof(string), string);
	DEBUG_PRINT("bigint 'big': %s", string);

	bigint_mul(&b3, &b1, &b2);

	bigint_tostr(&b3, sizeof(string), string);
	DEBUG_PRINT("bigint 'big': %s", string);

	/*bigint_long_mul(&b3, &b1, &b2);*/

	bigint_tostr(&b3, sizeof(string), string);
	DEBUG_PRINT("bigint 'big': %s", string);

	bigint_dump(&b1);
}

void test1(void) {
	BIGINT_BUFFER buf1[512];
	BIGINT_BUFFER buf2[512];
	BIGINT b1 = BIGINT_ASSIGN(buf1, sizeof(buf1));
	BIGINT b2 = BIGINT_ASSIGN(buf2, sizeof(buf2));

	memset(buf1, 0, sizeof(buf1));
	memset(buf2, 0, sizeof(buf2));

	getrandom(buf1, 256, 0);
	getrandom(buf2, 256, 0);

	bigint_dump(&b1);

	char str[bigint_tostr(&b1, 0, NULL)];

	bigint_tostr(&b1, sizeof(str), str);
	DEBUG_PRINT(str);
	bigint_tostr(&b2, sizeof(str), str);
	DEBUG_PRINT(str);

	bigint_mul(&b1, &b1, &b2);

	char prod[bigint_tostr(&b1, 0, NULL)];

	bigint_tostr(&b1, sizeof(prod), prod);

	DEBUG_PRINT(prod);
}

void testmul(void) {
	BIGINT_BUFFER buf1[512];
	BIGINT_BUFFER buf2[512];
	BIGINT b1 = BIGINT_ASSIGN(buf1, sizeof(buf1));
	BIGINT b2 = BIGINT_ASSIGN(buf2, sizeof(buf2));

	memset(buf1, 0, sizeof(buf1));
	memset(buf2, 0, sizeof(buf2));

	getrandom(buf1, 256, 0);
	getrandom(buf2, 256, 0);

	bigint_mul(&b1, &b1, &b2);
}

void test_longmul(void) {
	BIGINT a = {
		.buf = (BIGINT_BUFFER[6]) {0x00, 0xf0, 0xab, 0x75, 0xa4, 0x0d},
		.size = 6,
	};
	BIGINT b = {
		.buf = (BIGINT_BUFFER[6]) {0x00, 0xf0, 0xab, 0x75, 0xa4, 0x0d},
		.size = 6,
	};

	BIGINT c = {
		.buf = (BIGINT_BUFFER[12]) {0},
		.size = 12,
	};
	bigint_mul(&c, &a, &b);
}

void test2(void) {
	BIGINT_BUFFER buf[16] = {
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
	};
	BIGINT b = BIGINT_ASSIGN(buf, sizeof(buf));

	char str[64];

	bigint_tostr(&b, sizeof(str), str);

	DEBUG_PRINT(str);
}

void test3(void) {
	BIGINT a = {
		.buf = (BIGINT_BUFFER[6]) {0x00, 0xf0, 0xab, 0x75, 0xa4, 0x0d},
		.size = 6,
	};
	BIGINT b = {
		.buf = (BIGINT_BUFFER[6]) {0x00, 0xf0, 0xab, 0x75, 0xa4, 0x0d},
		.size = 6,
	};

	BIGINT c = {
		.buf = (BIGINT_BUFFER[12]) {0},
		.size = 12,
	};

	bigint_mul(&c, &a, &b);

	bigint_dump(&c);

	char str[bigint_tostr(&c, 0, NULL)];

	bigint_tostr(&c, sizeof(str), str);

	DEBUG_PRINT(str);
}



int main(void) {

	test3();
	/*test1();*/



	for(int i = 0; i < 500000; i++) {
		/*testmul();*/
		test_longmul();
	}

	return 0;
}
