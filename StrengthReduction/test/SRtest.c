int strength_red(int x){
    int r1 = x * 16;  //preciso
    int r2 = x * 15;  //diff<0
    int r3 = x * 10;  //diff>0
    int r4 = x * 47;  //no optimization
    return (r1+r2)+(r3+r4);
}