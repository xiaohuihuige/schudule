#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "net-common.h"

typedef struct
{
	atomic_int counter;
    uint8_t frame_type;       // 帧类型
    uint32_t timestamp;       // 时间戳
    uint32_t index;
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

	buffer->index  = 0;
	buffer->length = size;

	buffer->timestamp  = 0;
	buffer->frame_type = 0;
	buffer->counter    = 0;

	return buffer;
}

static inline void bufferReferenceCount(Buffer *buffer)
{
	if (!buffer)
		return;
	
	atomic_fetch_add(&buffer->counter, 1); // 原子加1
}


static inline void bufferReleaseSpace(Buffer *buffer)
{
	if (!buffer)
		return;

	atomic_fetch_sub(&buffer->counter, 1);

    if (0 == atomic_load(&buffer->counter))
		FREE(buffer);
}

static inline Buffer *createFrameBuffer(uint8_t *frame, size_t frame_len, uint8_t frame_type, uint32_t timestamp)
{
	Buffer * buffer = MALLOC(Buffer, sizeof(Buffer) + frame_len);
	if (!buffer) {
		ERR("malloc error");
		return NULL;
	}

	buffer->index  = 0;
	buffer->length = frame_len;

	buffer->timestamp  = timestamp;
	buffer->frame_type = frame_type;

	memcpy(buffer->data, frame, frame_len);

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


static inline Buffer* reinitializeBuffer(Buffer* buffer, const uint8_t* new_data, uint32_t new_length) 
{
	if (!buffer || !new_data)
		return NULL;

    uint32_t total_length = buffer->length + new_length;

    Buffer* new_buffer = (Buffer*)realloc(buffer, sizeof(Buffer) + total_length * sizeof(uint8_t));
    if (!new_buffer) 
        return NULL; 

    memcpy(buffer->data + buffer->length, new_data, new_length);

    buffer->length = total_length; // 更新长度

    return new_buffer; // 成功
}

#endif
