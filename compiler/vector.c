#include <stdio.h>
#include <stdlib.h>
#include "vector.h"

Vector* vec_new(int capacity){
    Vector *vec = (Vector*)malloc(sizeof(Vector));
    vec->data = (void**)malloc(sizeof(void*) * capacity);
    vec->capacity = capacity;
    vec->len = 0;
    return vec;
}

void *vec_push(Vector *vec, void* item) {
    if(vec->len >= vec->capacity) {
        vec_add_capacity(vec, 10);
    }
    vec->data[vec->len] = item;
    ++ vec->len;
    return item;
}

void *vec_pop(Vector *vec) {
    if(vec->len <= 0) {
        return NULL;
    }
    return vec->data[--vec->len];
}

void *vec_get(Vector *vec, int idx) {
    if(idx < 0 || vec->len <= idx){
        return NULL;
    }
    return vec->data[idx];
}

int vec_find(Vector *vec, void* target) {
    for(int idx = 0; idx < vec->len; ++ idx) {
        if(vec->data[idx] == target)
            return idx;
    }
    return -1;
}

void *vec_add_capacity(Vector *vec, int size) {
    int new_capacity = vec->capacity + size;
    void **new_data = (void**)malloc(sizeof(void*) * new_capacity);
    for(int idx = 0; idx < vec->capacity; ++ idx) {
        new_data[idx] = vec->data[idx];
    }
    free(vec->data);
    vec->capacity = new_capacity;
    vec->data = new_data;
}

void vec_free(Vector *vec) {
    free(vec->data);
    free(vec);
}
