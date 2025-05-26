void test_no_fusion() {
    int a[100], b[100];
    for (int i = 0; i < 100; ++i) {
        a[i] = i * 2;
        b[i] = i + 5;
    }
}

void test_fusion_direct() {
    int a[100], b[100];
    for (int i = 0; i < 100; ++i) {
        a[i] = i * 2;
    }
    for (int i = 0; i < 100; ++i) {
        b[i] = i + 5;
    }
}

void test_guard_case() {
    int a[100], b[100];
    volatile int condition = 1;
    
    if (condition) {
        for (int i = 0; i < 100; ++i) {
            a[i] = i * 2;
        }
    } else {
        for (int i = 0; i < 100; ++i) {
            b[i] = i + 5;
        }
    }
}


void test_non_adjacent() {
    int a[100], b[100];
    for (int i = 0; i < 100; ++i) {
        a[i] = i * 2;
    }    
    if (a[0] > 0) {
        a[1] = 42;
    } else {
        a[1] = 24;
    }
    for (int i = 0; i < 100; ++i) {
        b[i] = i + 5;
    }
}

void test_different_bounds() {
    int a[100], b[50];
    for (int i = 0; i < 100; ++i) {
        a[i] = i * 2;
    }
    for (int i = 0; i < 50; ++i) {
        b[i] = i + 5;
    }
}

void test_different_types() {
    int a[100];
    double b[100];
    for (int i = 0; i < 100; ++i) {
        a[i] = i * 2;
    }
    for (double i = 0; i < 100; ++i) {
        b[(int)i] = i + 5.0;
    }
}

void test_different_step() {
    int a[100], b[100];
    for (int i = 0; i < 100; i += 1) {
        a[i] = i * 2;
    }
    for (int i = 0; i < 100; i += 2) {
        b[i] = i + 5;
    }
}