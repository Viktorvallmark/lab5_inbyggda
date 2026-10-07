#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

typedef struct {
    uint32_t *data;
    uint32_t size;
    uint32_t head;
    uint32_t tail;
    uint32_t count;
} buffer_t;


void buffer_init(buffer_t *buffer, uint32_t size);
void buffer_add(buffer_t *buffer, uint32_t value);
uint32_t buffer_peek(buffer_t *buffer, int pos);
uint32_t buffer_pop(buffer_t *buffer);
uint32_t buffer_number_elements(buffer_t *buffer);
void buffer_print(buffer_t *buffer);
void buffer_free(buffer_t *buffer);



#endif
