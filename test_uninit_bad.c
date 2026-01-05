// Test case 1: obvious uninitialized variable use
int test_simple_uninit() {
    int x;
    int y = x + 1;  // ERROR: x is uninitialized
    return y;
}

// Test case 2: conditional initialization
int test_conditional_uninit() {
    int z;
    if (1) {
        z = 5;
    }
    return z;  // May be uninitialized if condition fails
}

// Test case 3: function use
void use_value(int val);

void test_param_uninit() {
    int a;
    use_value(a);  // ERROR: a is uninitialized
}

// Test case 4: multiple uses
int test_multiple_uses() {
    int m, n;
    m = 10;
    n = m + 20;  // ERROR: n is not initialized, but m is
    return n;
}
