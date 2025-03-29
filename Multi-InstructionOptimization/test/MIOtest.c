int MIOoptimization(int a, int b)
{
    //casi con add-sub
    int add1 = a + 1;
    int sub1 = add1 - 1;
    //casi con sub-add
    int sub2 = a - 1;
    int add2 = sub2 + 1;
    //casi con add-add
    int add3 = b + 2;
    int add4 = add3 + (-2);
    int add5 = a + b;
    int add6 = add5 + (-b);
    //casi con sub-sub
    int sub3 = b - 2;
    int sub4 = sub3 - (-2);
    int sub5 = a - b;
    int sub6 = sub5 - (-b);
    //casi con mul-div
    int mul1 = a * 2;
    int div1 = mul1 / 2;
    //casi con div-mul
    int div2 = a / 2;
    int mul2 = div2 * 2;
    //mancano i casi di add-sub e sub-add con le variabili
    //eventualmente per l'orale
    return sub1 + add2 + add4 + add6 + sub4 + sub6 + div1 + mul2;
}