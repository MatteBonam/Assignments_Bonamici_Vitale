void test_no_fusion() {
    int a[100], b[100];
    for (int i = 0; i < 100; ++i) {
        a[i] = i * 2;
        b[i] = i + 5;
    }
}

void test_fusion() {
    int a[100], b[100];
    for (int i = 0; i < 100; ++i)
        a[i] = i * 2;

    for (int i = 0; i < 100; ++i)
        b[i] = i + 5;
}
