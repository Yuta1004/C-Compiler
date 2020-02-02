#ifndef MY_VECTOR_H
#define MY_VECTOR_H

typedef struct Vector Vector;
struct Vector{
    void **data;
    int capacity;
    int len;
};

Vector *vec_new(int capacity);
void *vec_push(Vector *vec, void *item);
void *vec_pop(Vector *vec);
void *vec_get(Vector *vec, int idx);
int vec_find(Vector *vec, void* target);
void *vec_add_capacity(Vector *vec, int size);
void vec_free(Vector *free);

#endif // !MY_VECTOR_H