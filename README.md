# bigint

A bigint implementation in c

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

/* Add a and b together and store the result in a. Note that both BIGINTs in
an operation need to be of the same size */
bigint_add(a, b, sizeof(a));

bigint_mul(a, b, sizeof(a));
bigint_div(a, b, sizeof(a));
```

# TODO
- [ ] Speed up multiplication
- - [ ] Implement Karatsuba multiplication
- - [ ] Switch algorithm based on size of operands
- [ ] Add features to `bigint_set()`
- - [ ] Support hexadecimal input
- - [ ] Support negatives
- - [ ] support decimal places in scientific notation, e.g. `1.5e3`
- - [ ] Implement `bigint_validate_str()`
- - [ ] Implement `bigint_set()` without memory allocation
- - - [ ] Implement a function that can raise 10 to a power of n without allocating
- [ ] Add Benchmarking support
