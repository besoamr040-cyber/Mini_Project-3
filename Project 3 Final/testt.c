#include "dlmall.h"
#include <stdio.h>

int main() {
    int *a = (int *)dalloc(sizeof(int) * 10);
    for (int i = 0; i < 10; i++)
        a[i] = i * 2;

    for (int i = 0; i < 10; i++)
        printf("a[%d] = %d\n", i, a[i]);

    dfree(a);

    return 0;
}
