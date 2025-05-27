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

void test_control_flow_equivalent() {
    int a[100], b[100];
    int i;
    
    // Questi due loop sono control flow equivalent perché:
    // 1. Il primo loop domina il secondo (deve essere eseguito prima)
    // 2. Il secondo loop post-domina il primo (deve essere eseguito dopo)
    // Non c'è modo di eseguirne solo uno dei due
    
    for (i = 0; i < 100; ++i) {
        a[i] = i * 2;
    }
    
    for (i = 0; i < 100; ++i) {
        b[i] = a[i] + 5;  // dipende dal primo loop
    }
}

void test_not_control_flow_equivalent() {
    int a[100], b[100];
    int i;
    volatile int condition = 1;
    
    // Questi loop NON sono control flow equivalent perché:
    // Il secondo loop potrebbe non essere eseguito (condizione falsa)
    // Quindi non post-domina il primo
    
    for (i = 0; i < 100; ++i) {
        a[i] = i * 2;
    }
    
    if (condition) {
        for (i = 0; i < 100; ++i) {
            b[i] = a[i] + 5;
        }
    }
}

void test_negative_distance() {
    int a[100];
    int b[100];

    for (int i = 0; i < 98; ++i) {
        a[i] = i * 2; 
    }
   
    for (int i = 0; i < 98; ++i) {
        b[i] = a[i+3] + 1;  
    }
}