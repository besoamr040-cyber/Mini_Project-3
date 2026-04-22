#include <stdio.h>
#include <stdlib.h>

#define MEMORY 100

int memory[MEMORY];

typedef struct array {
    int size;
    int *segment;
    struct array *next;
} array;

array sentinel = {0, &memory[MEMORY], NULL};
array dummy = {1, &memory[-1], &sentinel};
array *allocated = &dummy;

void check() {
    array *nxt = allocated;
    while (nxt != NULL) {
        printf("array (%p): size %2d, segment %3ld, next %p\n",
               (void *)nxt, nxt->size, nxt->segment - memory, (void *)nxt->next);
        nxt = nxt->next;
    }
}

array *allocate(int size) {
    array *nxt = allocated;

    while (nxt->size != 0) {
        if ((nxt->next->segment - (nxt->segment + nxt->size)) >= size) {
            array *new = (array *)malloc(sizeof(array));
            new->size = size;
            new->segment = nxt->segment + nxt->size;
            new->next = nxt->next;
            nxt->next = new;
            return new;
        }
        nxt = nxt->next;
    }
    return NULL;
}

void compact() {
    array *prev = allocated;
    array *nxt = prev->next;

    while (nxt->size != 0) {
        for (int i = 0; i < nxt->size; i++) {
            prev->segment[prev->size + i] = nxt->segment[i];
        }
        nxt->segment = prev->segment + prev->size;
        prev = nxt;
        nxt = nxt->next;
    }
}

array *create(int size) {
    printf("create an array of size %4d\n", size);
    array *new = allocate(size);
    if (new == NULL) {
        printf("... almost panic, time for gc\n");
        compact();
        check();
        new = allocate(size);
    }
    printf("... pray for the best...\n");
    if (new == NULL) {
        printf("panic! memory full\n");
        exit(-1);
    }
    printf("... yes!\n");
    return new;
}

void delete(array *arr) {
    printf("delete array (%p) of size %4d\n", (void *)arr, arr->size);
    array *nxt = allocated;
    while (nxt->next != arr) {
        nxt = nxt->next;
    }
    nxt->next = arr->next;
    free(arr);
    printf("done\n");
}

void set(array *arr, int pos, int val) {
    if (pos <= 0 || pos > arr->size) {
        printf("segmentation fault (invalid write)\n");
        exit(-1);
    }
    arr->segment[pos - 1] = val;
}

int get(array *arr, int pos) {
    if (pos <= 0 || pos > arr->size) {
        printf("segmentation fault (invalid read)\n");
        exit(-1);
    }
    return arr->segment[pos - 1];
}

void bench1() {
    check();
    array *a = create(20);
    check();
    array *b = create(30);
    check();
    set(a, 10, 110);
    set(a, 14, 114);
    set(b, 8, 208);
    set(b, 12, 212);
    printf("a[10] + a[14] = %d\n", get(a, 10) + get(a, 14));
    printf("b[8] + b[12] = %d\n", get(b, 8) + get(b, 12));
    delete(a);
    check();
    delete(b);
    check();
}

int main() {
    bench1();
    return 0;
}
