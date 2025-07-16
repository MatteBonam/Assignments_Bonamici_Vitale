int test(int c, int f) {
    int b = 5 + f;
    int a, d, e, i = 0;
    //e = b + 3; se abbiamo i >= 3 non 
    //deve esser eseguita!
    while (i < 5) {
        a = b + c; //LI

        if (i < 3) {
            e = b + 3;  // skip non movable (exit block)
            break;
        } else {
            e = a + 4;  // neanche questa
        }

        d = a + 1;     // LI
        f = e + 2;     // no loop inv
        i++;           // PHInode non spostabile
    }

    return e + f;
}


int fun(int a, int b, int c){
    int i = 0;
    int d;
    int e;

    int z = 1;

    while(1){
        if(i<5)
            a = b + c; // more reaching definitions
        else{
            e = b + 1; 
            break;
        }
        d = a + 1; // a not moved, so d not movable
        i++;
    }

    c = a;  
    d = e;

    return c;

}

int main(){
    int a = fun(1,2,3);
    return a;
}