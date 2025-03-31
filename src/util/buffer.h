#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "net-common.h"

typedef struct
{
    uint32_t length;
    uint8_t  data[];
} Buffer;

static inline Buffer *createBuffer(size_t size)
{
	Buffer * buffer = MALLOC(Buffer, sizeof(Buffer) + size);
	if (!buffer) {
		ERR("malloc error");
		return NULL;
	}

	buffer->length = size;

	return buffer;
}

static inline int writeBuffer(Buffer *buffer, int index, uint8_t *src, size_t size)
{
	if (!buffer || !src || size <= 0)
		return -1;

	if (index >= buffer->length || index < 0)
		return -1;

    int actual_len = size;

    if (buffer->length - index < actual_len) { actual_len = buffer->length - index; }
    if (actual_len < 0) { actual_len = 0; }
    memcpy(buffer->data + index, src, actual_len);
    return actual_len;
}

static inline void writeBufferU1(Buffer *buffer, int index, int pos, uint32_t value)
{
	if (!buffer)
		return;

	if (index >= buffer->length || index < 0)
		return;

	if (pos <= 0 || pos > 8)
		return;

	(*(buffer->data + index)) &= ~(0x01 << pos);
    (*(buffer->data + index)) |= ((value & 0x01) << pos);	
}

static inline void writeBufferU(Buffer *buffer, int index, int pos, int n, uint32_t value)
{
	if (!buffer)
		return;

	if (index >= buffer->length || index < 0)
		return;

	if (pos <= 0 || pos > 8)
		return;

	int bits_left = pos;
	int indexs    = index;

	for (int i = 0; i < n; i++) {
		bits_left--;
		writeBufferU1(buffer, indexs, bits_left, (value >> ( n - i - 1 ))&0x01);
		if (bits_left == 0) { indexs++; bits_left = 8;}
	}
}


#endif
