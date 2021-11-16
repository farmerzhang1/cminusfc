void test(int a[]) {
    int i = a[3];
    return;
}
void b(int a){
    a=3;
    return;
}
int main(void) {
    int a[10];
    a[3] = 10;
    test(a);
    return 0;
}
