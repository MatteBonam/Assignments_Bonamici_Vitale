int strength_red(int x){
    int m1 = x * 16;  //preciso
    int m2 = x * 15;  //diff<0
    int m3 = x * 10;  //diff>0
    int m4 = x * 47;  //no optimization

    int d1 = x / 16;  //preciso
    int d2 = x / 15;  //no opt
    return (m1+m2+m3+m4)+(d1+d2);
}
