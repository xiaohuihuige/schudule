#ifndef __BUFFER_READER_H__
#define __BUFFER_READER_H__

#include "net-common.h"

typedef struct {
    uint8_t *buffer;
    uint32_t size;
    size_t r;
	size_t w;
} buffer_t, *buffer_ptr;

buffer_ptr buffer_init(uint32_t size);
void buffer_unint(buffer_ptr b);

int buffer_read(buffer_ptr b, int sockfd);
int buffer_fread(buffer_ptr b, FILE *fp);

uint8_t *begin_read(buffer_ptr b);
uint8_t *begin_write(buffer_ptr b);

uint32_t get_buffer_size(buffer_ptr b);

void retrieve(buffer_ptr b, size_t len);
void retrieve_all(buffer_ptr b);

uint32_t writable_bytes(buffer_ptr b);
uint32_t readable_bytes(buffer_ptr b);

#endif // !__BUFFER_READER_H__
