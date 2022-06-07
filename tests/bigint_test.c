#include <stdio.h>
/*#include <sys/random.h>*/

/*#include <gperftools/profiler.h>*/

#define __DEBUG_OUT stdout

#include "bigint.h"

int main(void) {

	/* TODO this should be the usage */
	BIGINT a[64];
	bigint_set(a, "25e100", sizeof(a));

	bigint_print(a, sizeof(a)); /* --> prints a 1 with 150 zeros */
	_bigint_dbgprint(a, sizeof(a));

	return 0;
}
