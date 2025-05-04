void loop_invariant_example(int n) {
    int a = 5;
    int b = 10;
    int c;
    int d = 5;
    // a*b is loop invariant and should be moved out
    for (int i = 0; i < n; i++) {
        c = a * b;  // This should be moved before the loop
        d += c;
    }
}