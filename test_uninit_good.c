// Test case 1: properly initialized
int test_proper_init() {
    int x = 0;
    int y = x + 1;  // OK: x is initialized
    return y;
}

// Test case 2: assignment before use
int test_assign_before_use() {
    int z;
    z = 5;
    return z;  // OK: z is assigned before use
}

// Test case 3: multiple variables
int test_multi_vars() {
    int a = 10;
    int b = 20;
    int c = a + b;  // OK: a and b are initialized
    return c;
}

// Test case 4: array and pointer (simplified - not fully analyzed)
void test_complex() {
    int arr[10];
    int *ptr = &arr[0];  // OK: ptr is initialized
}
