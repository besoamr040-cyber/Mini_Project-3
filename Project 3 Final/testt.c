#include "dlmall.h"
#include <stdio.h>

int main() {

    printf("=== Test 1: Basic Allocation ===\n");

    int *a = (int *)dalloc(sizeof(int) * 10);

    for (int i = 0; i < 10; i++)
        a[i] = i * 2;

    for (int i = 0; i < 10; i++)
        printf("a[%d] = %d\n", i, a[i]);

    printf("\n=== Freeing a ===\n");
    dfree(a);


    printf("\n=== Test 2: Multiple Allocations ===\n");

    int *b = (int *)dalloc(sizeof(int) * 20);
    int *c = (int *)dalloc(sizeof(int) * 30);

    printf("Allocated b and c\n");

    dfree(b);
    printf("Freed b (creates free space)\n");

    int *d = (int *)dalloc(sizeof(int) * 10);
    printf("Allocated d (should reuse free space from b)\n");


    printf("\n=== Test 3: Coalescing ===\n");

    dfree(c);
    dfree(d);

    printf("Freed c and d (should merge free blocks)\n");

    int *e = (int *)dalloc(sizeof(int) * 40);
    printf("Allocated e (after merging blocks)\n");

    dfree(e);

    printf("\n=== Done ===\n");

    return 0;
}
