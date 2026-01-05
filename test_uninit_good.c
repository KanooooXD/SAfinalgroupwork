// Good versions corresponding to the bad tests. Each variable is initialized
// so the checker should not warn while the bad file still triggers warnings.

// Test case 1: obvious uninitialized variable use (good: initialized)
int test_simple_uninit() {
    int x = 0;
    int y = x + 1;  // OK: x is initialized
    return y;
}

// Test case 2: conditional initialization (good: provide default)
int test_conditional_uninit() {
    int z = 0;
    if (1) {
        z = 5;
    }
    return z;  // OK: z has a default and may be overwritten
}

// Test case 3: function use (good: initialized before passing)
void use_value(int val);

void test_param_uninit() {
    int a = 0;
    use_value(a);  // OK: a is initialized
}

// Test case 4: multiple uses (good: initialize n)
int test_multiple_uses() {
    int m = 10, n = 0;
    n = m + 20;  // OK: n initialized from m
    return n;
}

// Additional good cases corresponding to extra bad tests

int test_simple_uninit_extra() {
    int x = 0;
    int y = x + 1;  // OK: x initialized
    return y;
}

int test_repeated_use_extra() {
    int u = 0;
    int v = u + u; // OK: u initialized
    return v;
}

int test_many_repeats_extra() {
    int d = 0;
    int e = d + d + d; // OK: d initialized
    return e;
}

int test_param_extra2();
void test_param_extra() {
    int a = 0;
    use_value(a);  // OK: a initialized
}

int test_multiple_extra() {
    int m = 10, n = 0;
    int p = n + m; // OK: n has default, p computed
    return p;
}

// Combined to reach many warnings in bad, but safe here
int test_combined() {
    int A = 0;
    int B = A + A; // OK: A initialized
    int C = 0;
    int D = C; // OK: C initialized
    return B + D;
}
