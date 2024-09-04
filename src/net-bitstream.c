#include "net-bitstream.h"

bs_t *bs_new(uint8_t *buf, size_t size)
{
    bs_t *b = (bs_t *)malloc(sizeof(bs_t));
    if (b == NULL)
    {
        return NULL;
    }

    b->start = buf;
    b->p = buf;
    b->end = buf + size;
    b->bits_left = 8;

    return b;
}

bs_t *bs_new_buffer(size_t size)
{
    uint8_t *buf = (uint8_t *)calloc(1, size);
    if (buf == NULL)
    {
        return NULL;
    }
    return bs_new(buf, size);
}

void bs_reset(bs_t *bs)
{
    if (bs == NULL)
        return;

    bs->p = bs->start;
    bs->bits_left = 8;
    return;
}

void bs_free(bs_t *b)
{
    net_free(b);
}

bs_t *bs_clone(bs_t *dest, const bs_t *src)
{
    dest->start = src->p;
    dest->p = src->p;
    dest->end = src->end;
    dest->bits_left = src->bits_left;
    return dest;
}

int bs_clone_bs(bs_t *dest, bs_t *src)
{
    if(dest == NULL || src == NULL)
        return -1;

    bs_write_bytes(dest, bs_start_ptr(src), bs_pos(src));
    
    return bs_pos(src) - 1;
}

uint32_t bs_byte_aligned(bs_t *b)
{
    return (b->bits_left == 8);
}

int bs_eof(bs_t *b)
{
    if (b->p >= b->end)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int bs_overrun(bs_t *b)
{
    if (b->p > b->end)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int bs_pos(bs_t *b)
{
    if (b->p > b->end)
    {
        return (b->end - b->start);
    }
    else
    {
        return (b->p - b->start);
    }
}

int bs_bytes_left(bs_t *b) 
{ 
    return (b->end - b->p);
}

uint32_t bs_read_u1(bs_t *b)
{
    uint32_t r = 0;

    b->bits_left--;

    if (!bs_eof(b))
    {
        r = ((*(b->p)) >> b->bits_left) & 0x01;
    }

    if (b->bits_left == 0)
    {
        b->p++;
        b->bits_left = 8;
    }

    return r;
}

uint32_t bs_read_ru1(bs_t *b)
{
    uint32_t r = 0;
    if (!bs_eof(b))
    {
        r = ((*(b->p)) >> b->bits_left) & 0x01;
    }
    return r;
}

void bs_skip_u1(bs_t *b)
{
    b->bits_left--;
    if (b->bits_left == 0)
    {
        b->p++;
        b->bits_left = 8;
    }
}

uint32_t bs_peek_u1(bs_t *b)
{
    uint32_t r = 0;

    if (!bs_eof(b))
    {
        r = ((*(b->p)) >> (b->bits_left - 1)) & 0x01;
    }
    return r;
}

uint32_t bs_read_u(bs_t *b, int n)
{
    uint32_t r = 0;
    int i;
    for (i = 0; i < n; i++)
    {
        r |= (bs_read_u1(b) << (n - i - 1));
    }
    return r;
}

uint32_t bs_read_ru(bs_t* b, int n)//读取n个字节,不移动
{
    uint32_t r = 0;
    int i;

    uint8_t *ptr = b->p;
    int temp_bits_left = b->bits_left;

    for (i = 0; i < n; i++) 
    {
        r |= (bs_read_u1(b) << (n - i - 1));
    } 

    b->p = ptr;
    b->bits_left = temp_bits_left;

    return r;
}

void bs_skip_u(bs_t *b, int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        bs_skip_u1(b);
    }
}

uint32_t bs_read_u8(bs_t *b)
{
    if (b->bits_left == 8 && !bs_eof(b)) // can do fast read
    {
        uint32_t r = b->p[0];
        b->p++;
        return r;
    }
    return bs_read_u(b, 8);
}

uint32_t bs_read_ue(bs_t *b)
{
    int32_t r = 0;
    int i = 0;

    while ((bs_read_u1(b) == 0) && (i < 32) && (!bs_eof(b)))
    {
        i++;
    }
    r = bs_read_u(b, i);
    r += (1 << i) - 1;
    return r;
}

int32_t bs_read_se(bs_t *b)
{
    int32_t r = bs_read_ue(b);
    if (r & 0x01)
    {
        r = (r + 1) / 2;
    }
    else
    {
        r = -(r / 2);
    }
    return r;
}

void bs_write_u1(bs_t *b, uint32_t v)
{
    b->bits_left--;

    if (!bs_eof(b))
    {
        (*(b->p)) &= ~(0x01 << b->bits_left);
        (*(b->p)) |= ((v & 0x01) << b->bits_left);
    }

    if (b->bits_left == 0)
    {
        b->p++;
        b->bits_left = 8;
    }
}

void bs_write_u(bs_t *b, int n, uint32_t v)
{
    int i;
    for (i = 0; i < n; i++)
    {
        bs_write_u1(b, (v >> (n - i - 1)) & 0x01);
    }
}

void bs_write_f(bs_t *b, int n, uint32_t v) { bs_write_u(b, n, v); }

void bs_write_u8(bs_t *b, uint32_t v)
{
    if (b->bits_left == 8 && !bs_eof(b)) // can do fast write
    {
        b->p[0] = v;
        b->p++;
        return;
    }
    bs_write_u(b, 8, v);
}

 void bs_write_ue(bs_t* b, uint32_t v)
{
    const int len_table[256] =
    {
        1,
        1,
        2,2,
        3,3,3,3,
        4,4,4,4,4,4,4,4,
        5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    };

    int len;

    if (v == 0)
    {
        bs_write_u1(b, 1);
    }
    else
    {
        v++;

        if (v >= 0x01000000)
        {
            len = 24 + len_table[ v >> 24 ];
        }
        else if(v >= 0x00010000)
        {
            len = 16 + len_table[ v >> 16 ];
        }
        else if(v >= 0x00000100)
        {
            len =  8 + len_table[ v >>  8 ];
        }
        else 
        {
            len = len_table[ v ];
        }

        bs_write_u(b, 2*len-1, v);
    }
}

void bs_write_se(bs_t *b, int32_t v)
{
    if (v <= 0)
    {
        bs_write_ue(b, -v * 2);
    }
    else
    {
        bs_write_ue(b, v * 2 - 1);
    }
}

int bs_read_bytes(bs_t *b, uint8_t *buf, int len)
{
    int actual_len = len;
    if (b->end - b->p < actual_len)
    {
        actual_len = b->end - b->p;
    }
    if (actual_len < 0)
    {
        actual_len = 0;
    }
    memcpy(buf, b->p, actual_len);
    if (len < 0)
    {
        len = 0;
    }
    b->p += len;
    return actual_len;
}

int bs_write_bytes(bs_t *b, uint8_t *buf, int len)
{
    int actual_len = len;
    if (b->end - b->p < actual_len)
    {
        actual_len = b->end - b->p;
    }
    if (actual_len < 0)
    {
        actual_len = 0;
    }
    memcpy(b->p, buf, actual_len);
    if (len < 0)
    {
        len = 0;
    }
    b->p += len;
    return actual_len;
}

int bs_skip_bytes(bs_t *b, int len)
{
    int actual_len = len;
    if (b->end - b->p < actual_len)
    {
        actual_len = b->end - b->p;
    }
    if (actual_len < 0)
    {
        actual_len = 0;
    }
    if (len < 0)
    {
        len = 0;
    }
    b->p += len;
    return actual_len;
}

uint32_t bs_next_bits(bs_t *bs, int nbits)
{
    bs_t b;
    bs_clone(&b, bs);
    return bs_read_u(&b, nbits);
}

uint64_t bs_next_bytes(bs_t *bs, int nbytes)
{
    int i = 0;
    uint64_t val = 0;

    if ((nbytes > 8) || (nbytes < 1))
    {
        return 0;
    }
    if (bs->p + nbytes > bs->end)
    {
        return 0;
    }

    for (i = 0; i < nbytes; i++)
    {
        val = (val << 8) | bs->p[i];
    }
    return val;
}

int bs_read_string(bs_t *bs, int str_size, uint8_t *string, int size)
{
    if (bs == NULL || string == NULL || str_size > size -1)
        return -1;

    if (bs_bytes_left(bs) <  str_size)
    {
        return -1;
    }

    for (int i = 0; i < str_size; i++)
    {
        string[i] = bs_read_u8(bs);
    }

    return str_size;
}

int bs_write_string(bs_t *bs, const char *string, int size)
{
    if (bs == NULL || string == NULL )
        return -1;

    if (bs_bytes_left(bs) <  size)
    {
        return -1;
    }

    for (int i = 0; i < size; i++)
    {
        bs_write_u8(bs, string[i]);
    }

    return size;
}

uint8_t *bs_start_ptr(bs_t *b)
{
    if (b == NULL)
        return NULL;
    return b->start;
}

uint8_t *bs_p_ptr(bs_t *b)
{
    if (b == NULL)
        return NULL;
    return b->p;
}
    
uint8_t *bs_end_ptr(bs_t *b)
{
    if (b == NULL)
        return NULL;
    return b->end;
}
