#include <stdio.h>
#include <stdlib.h>
int input() {
    int a;
    scanf("%d", &a);
    return a;
}

void output(int a) {
    printf("%d\n", a);
}

void outputFloat(float a) {
    printf("%f\n", a);
}

void neg_idx_except() {
    printf("negative index exception\n");
    exit(0);
}

void outputArray(int a[], int len) {
    printf("[");
    for (int i = 0; i < len; i++) printf("%d ", a[i]);
    printf("]\n");
}

void outputArrayf(float a[], int len) {
    printf("[ ");
    for (int i = 0; i < len; i++) printf("%f ", a[i]);
    printf("]\n");
}
