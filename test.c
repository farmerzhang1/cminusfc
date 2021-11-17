/*
     This code is adopted from Dik T. Winter at CWI
     It computes pi to 800 decimal digits
 */

int mod(int a, int b) {
    return a - a / b * b;
}

void printfour(int input) {
    int a;
    int b;
    int c;
    int d;
    input = mod(input, 10000);
    d = mod(input, 10);
    input = input / 10;
    c = mod(input, 10);
    input = input / 10;
    b = mod(input, 10);
    input = input / 10;
    a = input;
    printfour(a);
    printfour(b);
    printfour(c);
    printfour(d);
    return;
}

int main(void) {
    int r[2801];
    int i;
    int k;
    int b;
    int d;
    int c;
    c = 0;
    d = 1234;

    {
        int mod;
        mod = 0;
        while (mod < 2800) {
            r[mod] = 2000;
            mod = mod + 1;
        }
    }

    k = 2800;
    while (k) {
        int d;
        d = 0;
        i = k;

        while (i != 0) {
            d = d + r[i] * 10000;
            b = 2 * i - 1;
            r[i] = mod(d, b);
            d = d / b;
            i = i - 1;
            if (i != 0) {
                d = d * i;
            }
        }

        printfour(c + d / 10000);
        c = mod(d, 10000);

        k = k - 14;
    }

    return 0;
}
