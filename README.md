# bigint **WORK IN PROGRESS**

A bigint implementation in c (((**SUPER EXPERIMENTAL**)))

---

### Usage
```C
/* Define a bigint with a size of 64 bytes */
BIGINT a[64];

/* Set a to 10^150 */
bigint_set(a, "1e150", sizeof(a));

/* Prints a 1 with 150 zeros to stdout */
bigint_print(a, sizeof(a));

BIGINT b[64];

/* set 'b' to 65535 */
bigint_seti(b, 0xffff, sizeof(b));

BIGINT res[64];

/* Add a and b together and store the result in res. Note that all 3 BIGINTs in an operation need to be
of the same size */
bigint_add(res, a, b, sizeof(res));

bigint_mul(res, a, b, sizeof(res));
bigint_div(res, a, b, sizeof(res));

```
