#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>

void buffer_print(buffer_t *buffer) {
    for (uint32_t i = 0; i < buffer->count; i++) {
        printf("%lu ", (unsigned long)buffer_peek(buffer, i));
    }
    printf("\n");
}

void buffer_init(buffer_t *buffer, uint32_t size) {
    buffer->size = size;
    buffer->data = malloc(size * sizeof(uint32_t));
    for (int i = 0; i < size; i++) {
        buffer->data[i] = 0;
    }
    buffer->head = 0;
    buffer->tail = 0;
    buffer->count = 0;
}

void buffer_free(buffer_t *buffer) {
    free(buffer->data);
}

uint32_t buffer_number_elements(buffer_t *buffer) {
    return buffer->count;
}

void buffer_add(buffer_t *buffer, uint32_t value) {
    buffer->data[buffer->tail] = value;
    buffer->tail = (buffer->tail + 1) % buffer->size;
    if (buffer->count == buffer->size) {
        buffer->head = (buffer->head + 1) % buffer->size;
    } else {
        buffer->count++;
    }
}

uint32_t buffer_pop(buffer_t *buffer) {
    if (buffer->count == 0) return 0;
    uint32_t value = buffer->data[buffer->head];
    buffer->head = (buffer->head + 1) % buffer->size;
    buffer->count--;
    return value;
}

uint32_t buffer_peek(buffer_t *buffer, int pos) {
    return buffer->data[(buffer->head + pos) % buffer->size];
}
