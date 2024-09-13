#include "net-buffer.h"

static const uint32_t MAX_BYTES_PER_READ = 4096 * 512;
static const uint32_t MAX_BUFFER_SIZE = 1024 * 100000;
const uint8_t kCRLF[] = "\r\n";

buffer_ptr buffer_init(uint32_t size)
{
    buffer_ptr b = (buffer_ptr)calloc(1, sizeof(buffer_t)); 
    if (b == NULL)
        return NULL;

    b->buffer = (uint8_t *)malloc(size);
    if (b->buffer == NULL)
    {
        net_free(b);
        return NULL;
    }

    b->size = size;
    b->r = 0;
    b->w = 0;

    return b;
}

void buffer_unint(buffer_ptr b)
{
    if (b == NULL)
        return;
    net_free(b->buffer);
    net_free(b);
}

uint32_t readable_bytes(buffer_ptr b)
{
    if (b == NULL)
        return NET_FAIL;

    return (uint32_t)(b->w - b->r);
}

uint32_t writable_bytes(buffer_ptr b)
{
    if (b == NULL)
        return NET_FAIL;
    return (uint32_t)(b->size - b->w);
}

uint8_t *begin_write(buffer_ptr b)
{
    if (b == NULL)
        return NULL;
    return b->buffer + b->w;
}

uint8_t *begin_read(buffer_ptr b)
{
    if (b == NULL)
        return NULL;
    return b->buffer + b->r;
}

uint32_t get_buffer_size(buffer_ptr b)
{
    if (b == NULL)
        return NET_FAIL;
    return b->size;
}

void retrieve_all(buffer_ptr b)
{
    if (b == NULL)
        return;

    b->r = 0;
    b->w = 0;
}

void retrieve(buffer_ptr b, size_t len) 
{
    if (b == NULL)
        return;

    if (len <= readable_bytes(b))
    {
        b->r += len;
        if (b->r >= b->w)
        {
            b->r = 0;
            b->w = 0;
        }
    } else 
    {
        retrieve_all(b);
    }
}

int buffer_read(buffer_ptr b, SOCKET sockfd)
{
    if (b == NULL)
        return NET_FAIL;

    uint32_t size =  writable_bytes(b);
    if (size < MAX_BYTES_PER_READ)
    {
        uint32_t buffer_size = (uint32_t)b->size;
        if (buffer_size > MAX_BUFFER_SIZE)
            return NET_FAIL;

        uint8_t *new_buffer = (uint8_t *)realloc(b->buffer, buffer_size + MAX_BYTES_PER_READ);
        if (new_buffer == NULL)
            return NET_FAIL;
        b->buffer = new_buffer;
        b->size += MAX_BYTES_PER_READ;
    }

    int bytes_read = recv(sockfd, begin_write(b), MAX_BYTES_PER_READ, 0);
	if (bytes_read > 0) {
		b->w += bytes_read;
	}

    return bytes_read;
}

int buffer_fread(buffer_ptr b, FILE *fp)
{
    if (b == NULL || fp == NULL)
        return NET_FAIL;

    uint32_t size =  writable_bytes(b);
    if (size < MAX_BYTES_PER_READ)
    {
        uint32_t buffer_size = (uint32_t)b->size;
        if (buffer_size > MAX_BUFFER_SIZE)
            return NET_FAIL;

        uint8_t *new_buffer = (uint8_t *)realloc(b->buffer, buffer_size + MAX_BYTES_PER_READ);
        if (new_buffer == NULL)
            return NET_FAIL;
        b->buffer = new_buffer;
        b->size += MAX_BYTES_PER_READ;
    }

    int bytes_read = fread(begin_write(b), 1, MAX_BYTES_PER_READ, fp);
	if (bytes_read > 0) {
		b->w += bytes_read;
	}

    return bytes_read;
}